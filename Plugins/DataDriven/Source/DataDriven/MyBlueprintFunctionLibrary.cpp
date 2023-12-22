// Fill out your copyright notice in the Description page of Project Settings.


#include "MyBlueprintFunctionLibrary.h"
#include "DataAsset/MyPrimaryDataAsset.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/NoExportTypes.h"
#include "WorldSettings/MyWorldSettings.h"

TArray<FMyDataTable> UMyBlueprintFunctionLibrary::LoadMyDataTable()
{
	TArray<FMyDataTable> Res;
	UDataTable* DataTable = LoadObject<UDataTable>(nullptr, TEXT("/Script/Engine.DataTable'/DataDriven/DataTable/NewDataTable.NewDataTable'"));
	if (DataTable)
	{
		TArray<FMyDataTable*> Temp;
		FString ContextStr = TEXT("");
		DataTable->GetAllRows(ContextStr, Temp);
		for (FMyDataTable* Data : Temp)
		{
			Res.Add(*Data);
		}
	}
	
	return Res;
}

AMyWorldSettings* UMyBlueprintFunctionLibrary::GetMyWorldSettings(AActor* InActor)
{
	if (InActor->GetWorldSettings())
	{
		if (AMyWorldSettings* MyWorldSettings = Cast<AMyWorldSettings>(InActor->GetWorldSettings()))
		{
			return MyWorldSettings;
		}
	}
	return nullptr;
}

TArray<UObject*> UMyBlueprintFunctionLibrary::FindObjectsByClass(TArray<UClass*> InClass, bool bRecursivePaths/* = false*/)
{
	TArray<FString> PackagePaths;
	return FindObjectsByClassAndPaths(InClass, PackagePaths, bRecursivePaths);
}

TArray<UObject*> UMyBlueprintFunctionLibrary::FindObjectsByClassAndPaths(TArray<UClass*> InClass, TArray<FString> PackagePaths, bool bRecursivePaths /*= false*/)
{
	TArray<UObject*> Res;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> Data;
	FARFilter Filter;
	for (int32 Index = 0;Index < InClass.Num();Index++)
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
