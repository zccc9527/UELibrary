// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonUtilBPLibrary.h"
#include "CommonUtil.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Framework/Application/SlateApplication.h"

#if WITH_EDITOR
#include "Selection.h"
#include "Editor.h"
#endif

#include "Dom/JsonObject.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "UnrealExporter.h"
#include "Exporters/Exporter.h"
#include "Windows/WindowsPlatformApplicationMisc.h"
#include "GameFramework/DefaultPhysicsVolume.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Images/SImage.h"

class FSESelectedActorExportObjectInnerContext : public FExportObjectInnerContext
{
public:
	explicit FSESelectedActorExportObjectInnerContext(const TArray<AActor*> InSelectedActors) : FExportObjectInnerContext(false)
	{
		// For each selected actor...
		for (const AActor* Actor : InSelectedActors)
		{
			AddSelectedActor(Actor);
		}
	}

	virtual bool IsObjectSelected(const UObject* InObj) const override
	{
		const AActor* Actor = Cast<AActor>(InObj);
		return Actor && SelectedActors.Contains(Actor);
	}

private:

	void AddSelectedActor(const AActor* InActor)
	{
		SelectedActors.Add(InActor);

		ForEachObjectWithOuter(InActor, [this](UObject* InnerObj)
			{
				UObject* OuterObj = InnerObj->GetOuter();
				InnerList* Inners = ObjectToInnerMap.Find(OuterObj);
				if (Inners)
				{
					// Add object to existing inner list.
					Inners->Add(InnerObj);
				}
				else
				{
					// Create a new inner list for the outer object.
					InnerList& InnersForOuterObject = ObjectToInnerMap.Add(OuterObj, InnerList());
					InnersForOuterObject.Add(InnerObj);
				}
			}, /** bIncludeNestedObjects */ true, RF_NoFlags, EInternalObjectFlags::Garbage);

	}

	TSet<const AActor*> SelectedActors;
};

void ParentActors(AActor* ParentActor, AActor* ChildActor, const FName SocketName, USceneComponent* Component = nullptr)
{
	if (true/*CanParentActors(ParentActor, ChildActor)*/)
	{
		USceneComponent* ChildRoot = ChildActor->GetRootComponent();
		USceneComponent* ParentRoot = ParentActor->GetRootComponent();

		check(ChildRoot);	// CanParentActors() call should ensure this
		check(ParentRoot);	// CanParentActors() call should ensure this

		// modify parent and child
		//const FScopedTransaction Transaction(NSLOCTEXT("Editor", "UndoAction_PerformAttachment", "Attach actors"));
		// Attachment is persisted on the child so modify both actors for Undo/Redo but do not mark the Parent package dirty
		//ChildActor->Modify();
		//ParentActor->Modify(/*bAlwaysMarkDirty=*/false);

		// If child is already attached to something, modify the old parent and detach
		if (ChildRoot->GetAttachParent() != nullptr)
		{
			AActor* OldParentActor = ChildRoot->GetAttachParent()->GetOwner();
			//OldParentActor->Modify(/*bAlwaysMarkDirty=*/false);
			ChildRoot->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

			//GEngine->BroadcastLevelActorDetached(ChildActor, OldParentActor);
		}

		// If the parent is already attached to this child, modify its parent and detach so we can allow the attachment
		if (ParentRoot->IsAttachedTo(ChildRoot))
		{
			// Here its ok to mark the parent package dirty as both Parent & Child need to be saved.
			//ParentRoot->GetAttachParent()->GetOwner()->Modify();
			ParentRoot->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		}

		// Snap to socket if a valid socket name was provided, otherwise attach without changing the relative transform
		ChildRoot->AttachToComponent(Component ? Component : ParentRoot, FAttachmentTransformRules::KeepWorldTransform, SocketName);

		// Refresh editor in case child was translated after snapping to socket
		//RedrawLevelEditingViewports();
	}
}
bool GetFVECTOR(const TCHAR* Stream, FVector& Value)
{
	int32 NumVects = 0;

	Value = FVector::ZeroVector;

	// Support for old format.
	NumVects += FParse::Value(Stream, TEXT("X="), Value.X);
	NumVects += FParse::Value(Stream, TEXT("Y="), Value.Y);
	NumVects += FParse::Value(Stream, TEXT("Z="), Value.Z);

	// New format.
	if (NumVects == 0)
	{
		Value.X = (FVector::FReal)FCString::Atof(Stream);
		Stream = FCString::Strchr(Stream, ',');
		if (!Stream)
		{
			return 0;
		}

		Stream++;
		Value.Y = (FVector::FReal)FCString::Atof(Stream);
		Stream = FCString::Strchr(Stream, ',');
		if (!Stream)
		{
			return 0;
		}

		Stream++;
		Value.Z = (FVector::FReal)FCString::Atof(Stream);

		NumVects = 3;
	}

	return NumVects == 3;
}
UObject* FactoryCreateText(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn, TArray<AActor*>& OutPastedActors)
{
	UWorld* World = GWorld;

	UPackage* RootMapPackage = Cast<UPackage>(InParent);
	TMap<FString, UPackage*> MapPackages;
	TMap<AActor*, AActor*> MapActors;
	FParse::Next(&Buffer);
	if (GetBEGIN2(&Buffer, TEXT("MAP")))
	{
		if (ULevel* Level = Cast<ULevel>(InParent))
		{
			World = Level->GetWorld();
		}

		if (RootMapPackage)
		{
			FString MapName;
			if (FParse::Value(Buffer, TEXT("Name="), MapName))
			{
				// Advance the buffer
				Buffer += FCString::Strlen(TEXT("Name="));
				Buffer += MapName.Len();
				// Check to make sure that there are no naming conflicts
				if (RootMapPackage->Rename(*MapName, nullptr, REN_Test | REN_ForceNoResetLoaders))
				{
					// Rename it!
					RootMapPackage->Rename(*MapName, nullptr, REN_ForceNoResetLoaders);
				}

				// Stick it in the package map
				MapPackages.Add(MapName, RootMapPackage);
			}
		}
	}
	else
	{
		return World;
	}

	bool bIsExpectingNewMapTag = false;
	bool bShouldSkipImportSpecialActors = false;

	FString MapPackageText;

	int32 ActorIndex = 0;


	// Maintain a list of a new actors and the text they were created from.
	TMap<AActor*, FString> NewActorMap;

	// Maintain a lookup for the new actors, keyed by their source FName.
	TMap<FName, AActor*> NewActorsFNames;

	// Maintain a lookup from existing to new actors, used when replacing internal references when copy+pasting / duplicating
	TMap<AActor*, AActor*> ExistingToNewMap;

	// Maintain a lookup of the new actors to their parent and socket attachment if provided.
	struct FAttachmentDetail
	{
		const FName ParentName;
		const FName SocketName;
		FAttachmentDetail(const FName InParentName, const FName InSocketName) : ParentName(InParentName), SocketName(InSocketName) {}
	};
	TMap<AActor*, FAttachmentDetail> NewActorsAttachmentMap;

	FString StrLine;
	while (FParse::Line(&Buffer, StrLine))
	{
		const TCHAR* Str = *StrLine;

		// If we're still waiting to see a 'MAP' tag, then check for that
		if (bIsExpectingNewMapTag)
		{
			if (GetBEGIN2(&Str, TEXT("MAP")))
			{
				bIsExpectingNewMapTag = false;
			}
			else
			{
				// Not a new map tag, so continue on
			}
		}
		else if (GetEND2(&Str, TEXT("MAP")))
		{
			// End of brush polys.
			bIsExpectingNewMapTag = true;
		}
		else if (GetBEGIN2(&Str, TEXT("ACTOR")))
		{
			UClass* TempClass;
			if (ParseObject<UClass>(Str, TEXT("CLASS="), TempClass, nullptr, EParseObjectLoadingPolicy::FindOrLoad))
			{
				// Get actor name.
				FName ActorUniqueName(NAME_None);
				FName ActorSourceName(NAME_None);
				FParse::Value(Str, TEXT("NAME="), ActorSourceName);
				ActorUniqueName = ActorSourceName;

				AActor* Found = nullptr;
				if (ActorUniqueName != NAME_None)
				{
					// look in the current level for the same named actor
					Found = FindObject<AActor>(World->GetCurrentLevel(), *ActorUniqueName.ToString());
				}

				// Make sure this name is unique. We need to do this upfront because we also want to potentially create the Associated BP class using the same name.
				//bool bNeedGloballyUniqueName = World->GetCurrentLevel()->IsUsingExternalActors() && CastChecked<AActor>(TempClass->GetDefaultObject())->SupportsExternalPackaging();
				ActorUniqueName = FActorSpawnUtils::MakeUniqueActorName(World->GetCurrentLevel(), TempClass, FActorSpawnUtils::GetBaseName(ActorUniqueName), true);

				// Get parent name for attachment.
				FName ActorParentName(NAME_None);
				FParse::Value(Str, TEXT("ParentActor="), ActorParentName);

				// Get socket name for attachment.
				FName ActorParentSocket(NAME_None);
				FParse::Value(Str, TEXT("SocketName="), ActorParentSocket);

				// if an archetype was specified in the Begin Object block, use that as the template for the ConstructObject call.
				FString ArchetypeName;
				AActor* Archetype = nullptr;
				if (FParse::Value(Str, TEXT("Archetype="), ArchetypeName))
				{
					// if given a name, break it up along the ' so separate the class from the name
					FString ObjectClass;
					FString ObjectPath;
					if (FPackageName::ParseExportTextPath(ArchetypeName, &ObjectClass, &ObjectPath))
					{
						// find the class
						UClass* ArchetypeClass = UClass::TryFindTypeSlow<UClass>(ObjectClass, EFindFirstObjectOptions::EnsureIfAmbiguous);
						if (ArchetypeClass)
						{
							if (ArchetypeClass->IsChildOf(AActor::StaticClass()))
							{
								// if we had the class, find the archetype
								Archetype = Cast<AActor>(StaticFindObject(ArchetypeClass, nullptr, *ObjectPath));
							}
							else
							{
								Warn->Logf(ELogVerbosity::Warning, TEXT("Invalid archetype specified in subobject definition '%s': %s is not a child of Actor"),
									Str, *ObjectClass);
							}
						}
					}
				}

				if (TempClass->IsChildOf(AWorldSettings::StaticClass()))
				{
					// if we see a WorldSettings, then we are importing an entire level, so if we
					// are importing into an existing level, then we should not import the next actor
					// which will be the builder brush
					check(ActorIndex == 0);

					// if we have any actors, then we are importing into an existing level
					if (World->GetCurrentLevel()->Actors.Num())
					{
						check(World->GetCurrentLevel()->Actors[0]->IsA(AWorldSettings::StaticClass()));

						// full level into full level, skip the first two actors
						bShouldSkipImportSpecialActors = true;
					}
				}

				// Get property text.
				FString PropText, PropertyLine;
				while (GetEND2(&Buffer, TEXT("ACTOR")) == 0 && FParse::Line(&Buffer, PropertyLine))
				{
					PropText += *PropertyLine;
					PropText += TEXT("\r\n");
				}

				// If we need to skip the WorldSettings and BuilderBrush, skip the first two actors.  Note that
				// at this point, we already know that we have a WorldSettings and BuilderBrush in the .t3d.
				/*if (FLevelUtils::IsLevelLocked(World->GetCurrentLevel()))
				{
					UE_LOG(LogEditorFactories, Warning, TEXT("Import actor: The requested operation could not be completed because the level is locked."));
					GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, nullptr);
					return nullptr;
				}*/
				if (!(bShouldSkipImportSpecialActors && ActorIndex < 2))
				{
					// Don't import the default physics volume, as it doesn't have a UModel associated with it
					// and thus will not import properly.
					if (!TempClass->IsChildOf(ADefaultPhysicsVolume::StaticClass()))
					{
						// Create a new actor.
						FActorSpawnParameters SpawnInfo;
						SpawnInfo.Name = ActorUniqueName;
						SpawnInfo.Template = Archetype;
						SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
						AActor* NewActor = World->SpawnActor(TempClass, nullptr, nullptr, SpawnInfo);

						if (NewActor)
						{
							OutPastedActors.Add(NewActor);

							//FTimerHandle TimerHandle;
							//FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &UWidgetDataManager::UpdateActorInOutliner, NewActor);
							//GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.05, false);

							// Store the new actor and the text it should be initialized with.
							NewActorMap.Add(NewActor, *PropText);

							// Store the copy to original actor mapping
							MapActors.Add(NewActor, Found);

							// Store the new actor against its source actor name (not the one that may have been made unique)
							if (ActorSourceName != NAME_None)
							{
								NewActorsFNames.Add(ActorSourceName, NewActor);
								if (Found)
								{
									ExistingToNewMap.Add(Found, NewActor);
								}
							}

							// Store the new actor with its parent's FName, and socket FName if applicable
							if (ActorParentName != NAME_None)
							{
								NewActorsAttachmentMap.Add(NewActor, FAttachmentDetail(ActorParentName, ActorParentSocket));
							}
						}
					}
				}

				// increment the number of actors we imported
				ActorIndex++;
			}
		}
		else if (GetBEGIN2(&Str, TEXT("SURFACE")))
		{
			UMaterialInterface* SrcMaterial = nullptr;
			FVector SrcBase, SrcTextureU, SrcTextureV, SrcNormal;
			uint32 SrcPolyFlags = PF_DefaultFlags;
			int32 SurfacePropertiesParsed = 0;

			SrcBase = FVector::ZeroVector;
			SrcTextureU = FVector::ZeroVector;
			SrcTextureV = FVector::ZeroVector;
			SrcNormal = FVector::ZeroVector;

			bool bJustParsedTextureName = false;
			bool bFoundSurfaceEnd = false;
			bool bParsedLineSuccessfully = false;

			do
			{
				if (GetEND2(&Buffer, TEXT("SURFACE")))
				{
					bFoundSurfaceEnd = true;
					bParsedLineSuccessfully = true;
				}
				else if (FParse::Command(&Buffer, TEXT("TEXTURE")))
				{
					Buffer++;	// Move past the '=' sign

					FString TextureName;
					bParsedLineSuccessfully = FParse::Line(&Buffer, TextureName, true);
					if (TextureName != TEXT("None"))
					{
						SrcMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *TextureName, nullptr, LOAD_NoWarn, nullptr));
					}
					bJustParsedTextureName = true;
					SurfacePropertiesParsed++;
				}
				else if (FParse::Command(&Buffer, TEXT("BASE")))
				{
					GetFVECTOR(Buffer, SrcBase);
					SurfacePropertiesParsed++;
				}
				else if (FParse::Command(&Buffer, TEXT("TEXTUREU")))
				{
					GetFVECTOR(Buffer, SrcTextureU);
					SurfacePropertiesParsed++;
				}
				else if (FParse::Command(&Buffer, TEXT("TEXTUREV")))
				{
					GetFVECTOR(Buffer, SrcTextureV);
					SurfacePropertiesParsed++;
				}
				else if (FParse::Command(&Buffer, TEXT("NORMAL")))
				{
					GetFVECTOR(Buffer, SrcNormal);
					SurfacePropertiesParsed++;
				}
				else if (FParse::Command(&Buffer, TEXT("POLYFLAGS")))
				{
					FParse::Value(Buffer, TEXT("="), SrcPolyFlags);
					SurfacePropertiesParsed++;
				}

				// Parse to the next line only if the texture name wasn't just parsed or if the 
				// end of surface isn't parsed. Don't parse to the next line for the texture 
				// name because a FParse::Line() is called when retrieving the texture name. 
				// Doing another FParse::Line() would skip past a necessary surface property.
				if (!bJustParsedTextureName && !bFoundSurfaceEnd)
				{
					FString DummyLine;
					bParsedLineSuccessfully = FParse::Line(&Buffer, DummyLine);
				}

				// Reset this bool so that we can parse lines starting during next iteration.
				bJustParsedTextureName = false;
			} while (!bFoundSurfaceEnd && bParsedLineSuccessfully);
		}
		else if (GetBEGIN2(&Str, TEXT("MAPPACKAGE")))
		{
			// Get all the text.
			while ((GetEND2(&Buffer, TEXT("MAPPACKAGE")) == 0) && FParse::Line(&Buffer, StrLine))
			{
				MapPackageText += *StrLine;
				MapPackageText += TEXT("\r\n");
			}
		}
	}

	//导入属性
	for (auto ActorMapElement : NewActorMap)
	{
		AActor* Actor = ActorMapElement.Key;
		FString PropertyText = ActorMapElement.Value;

		ImportObjectProperties2((uint8*)Actor, *PropertyText, Actor->GetClass(), Actor, Actor, Warn, 0, INDEX_NONE, NULL, &ExistingToNewMap);

		Actor->UpdateComponentTransforms();
	}

	TSet<AActor*> ParentActorSet;
	// Pass 2: Sort out any attachment parenting on the new actors now that all actors have the correct properties set
	for (auto It = MapActors.CreateIterator(); It; ++It)
	{
		AActor* const Actor = It.Key();

		// Fixup parenting
		FAttachmentDetail* ActorAttachmentDetail = NewActorsAttachmentMap.Find(Actor);
		if (ActorAttachmentDetail != nullptr)
		{
			AActor* ActorParent = nullptr;
			// Try to find the new copy of the parent
			AActor** NewActorParent = NewActorsFNames.Find(ActorAttachmentDetail->ParentName);
			if (NewActorParent != nullptr)
			{
				ActorParent = *NewActorParent;
			}
			// Try to find an already existing parent
			if (ActorParent == nullptr)
			{
				ActorParent = FindObject<AActor>(World->GetCurrentLevel(), *ActorAttachmentDetail->ParentName.ToString());
			}
			// Parent the actors
			if (GWorld == World && ActorParent != nullptr)
			{
				if (!ParentActorSet.Contains(ActorParent))
				{
					UStaticMeshComponent* StaticMeshComponent = NewObject<UStaticMeshComponent>(ActorParent, TEXT("StaticMeshComponent"));
					USceneComponent* ParentRoot = ActorParent->GetRootComponent();
					StaticMeshComponent->AttachToComponent(ParentRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
					ParentActorSet.Add(ActorParent);
				}

				ParentActors(ActorParent, Actor, ActorAttachmentDetail->SocketName);
			}
		}
	}

	return World;
}



UWorld* UCommonUtilBPLibrary::ForEachWorld(TFunction<bool(UWorld*)> InTriggerFunc)
{
	for (const FWorldContext WorldContext : GEngine->GetWorldContexts())
	{
		if (InTriggerFunc(WorldContext.World()))
		{
			return WorldContext.World();
		}
	}
	return nullptr;
}

TArray<UObject*> UCommonUtilBPLibrary::FindObjectsByClass(TArray<UClass*> InClass, bool bRecursivePaths /*= false*/)
{
	TArray<FString> PackagePaths;
	return FindObjectsByClassAndPaths(InClass, PackagePaths, bRecursivePaths);
}

TArray<UObject*> UCommonUtilBPLibrary::FindObjectsByClassAndPaths(TArray<UClass*> InClass, TArray<FString> PackagePaths, bool bRecursivePaths /*= false*/)
{
	TArray<UObject*> Res;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> Data;
	FARFilter Filter;
	for (int32 Index = 0; Index < InClass.Num(); Index++)
	{
		Filter.ClassPaths.Add(InClass[Index]->GetClassPathName());
	}
	Filter.bRecursivePaths = bRecursivePaths;
	Filter.PackagePaths.Append(PackagePaths);
	AssetRegistryModule.Get().GetAssets(Filter, Data);

	for (FAssetData AssetData : Data)
	{
		if (AssetData.GetAsset())
		{
			Res.Add(AssetData.GetAsset());
		}
	}
	return Res;
}

TSharedPtr<SWidget> UCommonUtilBPLibrary::GetSlateWidgetUnderCursor()
{
	return GetSlateWidgetOnPosition(FSlateApplication::Get().GetCursorPos());
}

TSharedPtr<SWidget> UCommonUtilBPLibrary::GetSlateWidgetOnPosition(FVector2D InPosition)
{
	if (FSlateApplication::IsInitialized())
	{
		TSharedPtr<SWindow> Window = FSlateApplication::Get().GetActiveTopLevelWindow();
		if (!Window.IsValid())
		{
			return nullptr;
		}
		TArray<TSharedRef<SWindow>> Windows;
		Windows.Add(Window.ToSharedRef());
		FWidgetPath WidgetPath = FSlateApplication::Get().LocateWindowUnderMouse(InPosition, Windows);
		if (WidgetPath.Widgets.Num() > 0)
		{
			TSharedPtr<SWidget> PositionWidget = WidgetPath.Widgets.Last().Widget;

			return PositionWidget;
		}
	}
	return nullptr;
}

TArray<FString> UCommonUtilBPLibrary::FindFiles(const FString& InPath, const FString& Extension, bool bRecursive/* = false*/, bool bDirectories/* = false*/)
{
	TArray<FString> OutFiles;
	if (bRecursive)
	{
		IFileManager::Get().FindFilesRecursive(OutFiles, *InPath, *(TEXT("*.") + Extension), true, bDirectories);
	}
	else
	{
		IFileManager::Get().FindFiles(OutFiles, *InPath, *(TEXT("*.") + Extension));
	}
	return OutFiles;
}

void UCommonUtilBPLibrary::AddWidgetToViewport(TSharedPtr<SWidget> InWidget, FVector2D InPosition, FVector2D InSize, FVector2D InAlignment, int32 InZOrder/* = -1*/)
{
	UWorld* CurrentWorld = GWorld->GetWorld();
	check(CurrentWorld && CurrentWorld->IsGameWorld());
	UGameViewportClient* ViewportClient = CurrentWorld->GetGameViewport();
	check(ViewportClient);

	TSharedPtr<SConstraintCanvas> RenderCanvas = SNew(SConstraintCanvas);
	RenderCanvas->AddSlot().Offset(FMargin(InPosition.X, InPosition.Y, InSize.X, InSize.Y)).Alignment(InAlignment)
		[
			InWidget.ToSharedRef()
		];
	ViewportClient->AddViewportWidgetContent(RenderCanvas.ToSharedRef(), InZOrder);
}

void UCommonUtilBPLibrary::AddWidgetToViewport2(FVector2D InPosition, FVector2D InSize, FVector2D InAlignment, int32 InZOrder /*= -1*/)
{
	TSharedPtr<SImage> Image = SNew(SImage);
	AddWidgetToViewport(Image, InPosition, InSize, InAlignment, InZOrder);
}

#if WITH_EDITOR
TArray<UObject*> UCommonUtilBPLibrary::GetCurrentSelectedObjects()
{
	TArray<UObject*> Res;
	GEditor->GetSelectedActors()->GetSelectedObjects(Res);
	return Res;
}

TArray<AActor*> UCommonUtilBPLibrary::GetCurrentSelectedActors()
{
	TArray<AActor*> Res;
	GEditor->GetSelectedActors()->GetSelectedObjects(Res);
	return Res;
}
#endif

void UCommonUtilBPLibrary::CopyActors(TArray<AActor*> Actors, FString* DestinationData /*= nullptr*/)
{
	if (Actors.IsEmpty()) return;

	UWorld* World = Actors[0]->GetWorld();
	if (World)
	{
		// 导出Actor数据
		FStringOutputDevice Ar;
		const FSESelectedActorExportObjectInnerContext Context(Actors);
		UExporter::ExportToOutputDevice(&Context, World, NULL, Ar, TEXT("copy"), 0, PPF_DeepCompareInstances | PPF_ExportsNotFullyQualified);
		if (DestinationData)
		{
			*DestinationData = MoveTemp(Ar);
		}
		else
		{
			FPlatformApplicationMisc::ClipboardCopy(*Ar);
		}
	}
}

void UCommonUtilBPLibrary::PasteActors(TArray<AActor*>& OutPastedActors, FString* SourceData /*= nullptr*/)
{
	OutPastedActors.Reset();

	auto FindWorld = [](UWorld* InWorld)->bool
	{
		#if WITH_EDITOR || GIsEditor
		if (InWorld->WorldType == EWorldType::PIE)	return true;
		#else
		if (InWorld->WorldType == EWorldType::Game || InWorld->WorldType == EWorldType::GamePreview) return true;
		#endif
		return false;
	};

	UWorld* World = UCommonUtilBPLibrary::ForEachWorld(FindWorld);
	if (World == nullptr)
	{
		World = UCommonUtilBPLibrary::ForEachWorld([](UWorld* InWorld){ return InWorld->WorldType == EWorldType::Editor; });
	}

	//从粘贴板中获取文本?
	FString PasteString;
	if (SourceData)
	{
		PasteString = *SourceData;
	}
	else
	{
		FPlatformApplicationMisc::ClipboardPaste(PasteString);
	}

	const TCHAR* Paste = *PasteString;

	FactoryCreateText(ULevel::StaticClass(), World->GetCurrentLevel(), World->GetCurrentLevel()->GetFName(), RF_Transactional, NULL, TEXT("paste"), Paste, Paste + FCString::Strlen(Paste), GWarn, OutPastedActors);
}

void UCommonUtilBPLibrary::CopyActorsTest(TArray<AActor*> Actors)
{
	CopyActors(Actors);
}

void UCommonUtilBPLibrary::PasteActorsTest(TArray<AActor*> OutPastedActors)
{
	PasteActors(OutPastedActors);
}
