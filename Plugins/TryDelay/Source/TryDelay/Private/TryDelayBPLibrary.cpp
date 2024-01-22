// Copyright Epic Games, Inc. All Rights Reserved.

#include "TryDelayBPLibrary.h"
#include "DelayAction.h"
#include "UObject/NoExportTypes.h"
#include "UObject/SavePackage.h"

void UTryDelayBPLibrary::DelayFunctionName(UObject* CallbackTarget, int32 uuid, FName ExecutionFunction, float Duration, bool bRetriggerable/* = true*/)
{
	if (UWorld* World = CallbackTarget->GetWorld())
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();

		static int32 NewId = 0;
		if (uuid == -1)
		{
			uuid = ++NewId;
		}

		FLatentActionInfo LatentActionInfo;
		LatentActionInfo.CallbackTarget = CallbackTarget;
		LatentActionInfo.ExecutionFunction = ExecutionFunction;
		LatentActionInfo.Linkage = 0;
		LatentActionInfo.UUID = uuid;

		FDelayAction* DelayAction = LatentActionManager.FindExistingAction<FDelayAction>(LatentActionInfo.CallbackTarget, LatentActionInfo.UUID);
		if (DelayAction == nullptr)
		{
			LatentActionManager.AddNewAction(LatentActionInfo.CallbackTarget, LatentActionInfo.UUID, new FDelayAction(Duration, LatentActionInfo));
		}
		else
		{
			if (bRetriggerable)
			{
				DelayAction->TimeRemaining = Duration;
			}
		}

		NewId = uuid;
	}
}

void UTryDelayBPLibrary::DelayLambda(int32 uuid, float Duration, TFunction<void()> InTriggerFunc, bool bRetriggerable/* = true*/)
{
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
		return;

	static int32 NewId = 0;
	if (uuid == -1)
	{
		uuid = ++NewId;
	}

	FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
	FTFunctionDelayAction* LambdaDelayAction = LatentActionManager.FindExistingAction<FTFunctionDelayAction>(World, uuid);
	if (LambdaDelayAction == nullptr)
	{
		LatentActionManager.AddNewAction(World, uuid, new FTFunctionDelayAction(Duration, InTriggerFunc));
	}
	else
	{
		if (bRetriggerable)
		{
			LambdaDelayAction->SetDuration(Duration);
		}
	}
	NewId = uuid;
}

//template<class C, typename... Args>
//void UTryDelayBPLibrary::DelayMemberFunction(UObject* Obj, int32 uuid, float Duration, bool bRetriggerable, void(C::* pf)(Args...), Args... args)
//{
//	UWorld* World = Obj->GetWorld();
//	if (World == nullptr)
//		return;
//
//	static int32 NewId = 0;
//	if (uuid == -1)
//	{
//		uuid = ++NewId;
//	}
//
//	FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
//
//	FObjectDelayAction<C, Args...>* ObjectDelayAction = LatentActionManager.FindExistingAction<FObjectDelayAction<C, Args...>>(Obj, uuid);
//	if (ObjectDelayAction == nullptr)
//	{
//		LatentActionManager.AddNewAction(Obj, uuid, new FObjectDelayAction<C, Args...>(Duration, Cast<C>(Obj), pf, args...));
//	}
//	else
//	{
//		if (bRetriggerable)
//		{
//			ObjectDelayAction->SetDuration(Duration);
//		}
//	}
//	NewId = uuid;
//}

//template<class C, typename... Args>
//void UTryDelayBPLibrary::DelayRawFunction(C* Obj, int32 uuid, float Duration, bool bRetriggerable, void(C::* pf)(Args...), Args... args)
//{
//	auto FindWorld = [](UWorld* InWorld)->bool
//	{
//		#if WITH_EDITOR || GIsEditor
//		if (InWorld->WorldType == EWorldType::PIE)	return true;
//		#else
//		if (InWorld->WorldType == EWorldType::Game || InWorld->WorldType == EWorldType::GamePreview) return true;
//		#endif
//		return false;
//	};
//
//	UWorld* World = UCommonUtilBPLibrary::ForEachWorld(FindWorld);
//	if (World == nullptr) return;
//
//	FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
//
//	static int32 NewId = 0;
//	if (uuid == -1)
//	{
//		uuid = ++NewId;
//	}
//
//	FRawDelayAction<C, Args...>* ObjectDelayAction = LatentActionManager.FindExistingAction<FRawDelayAction<C, Args...>>(World, uuid);
//	if (ObjectDelayAction == nullptr)
//	{
//		LatentActionManager.AddNewAction(World, uuid, new FRawDelayAction<C, Args...>(Duration, Obj, pf, args...));
//	}
//	else
//	{
//		if (bRetriggerable)
//		{
//			ObjectDelayAction->SetDuration(Duration);
//		}
//	}
//	NewId = uuid;
//}
