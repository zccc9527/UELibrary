// Copyright Epic Games, Inc. All Rights Reserved.

#include "TryDelayBPLibrary.h"
#include "DelayAction.h"
#include "UObject/NoExportTypes.h"
#include "UObject/SavePackage.h"

void UTryDelayBPLibrary::DelayFunctionName(UObject* CallbackTarget, FName ExecutionFunction, float Duration, bool bRetriggerable/* = true*/)
{
	if (UWorld* World = CallbackTarget->GetWorld())
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();

		FLatentActionInfo LatentActionInfo;
		LatentActionInfo.CallbackTarget = CallbackTarget;
		LatentActionInfo.ExecutionFunction = ExecutionFunction;
		LatentActionInfo.Linkage = 0;
		LatentActionInfo.UUID = FMath::Rand();

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
	}
}

void UTryDelayBPLibrary::DelayLambda(UObject* Obj, int32 uuid, float Duration, TFunction<void()> InTriggerFunc, bool bRetriggerable/* = true*/)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(Obj, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();

		FLambdaDelayAction* LambdaDelayAction = LatentActionManager.FindExistingAction<FLambdaDelayAction>(Obj, uuid);
		if (LambdaDelayAction == nullptr)
		{
			LatentActionManager.AddNewAction(Obj, uuid, new FLambdaDelayAction(Duration, InTriggerFunc));
		}
		else
		{
			if (bRetriggerable)
			{
				LambdaDelayAction->SetDuration(Duration);
			}
		}
	}
}

//template<class C, typename Ret, typename... Args>
//void UTryDelayBPLibrary::DelayMemberFunction(C* Obj, int32 uuid, float Duration, Ret(C::* pf)(Args...), Args... args)
//{
//	if (UWorld* World = GEngine->GetWorldFromContextObject(Obj, EGetWorldErrorMode::LogAndReturnNull))
//	{
//		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
//
//		FObjectDelayAction<C, Ret, Args...>* ObjectDelayAction = LatentActionManager.FindExistingAction<FObjectDelayAction<C, Ret, Args...>>(Obj, uuid);
//		if (ObjectDelayAction == nullptr)
//		{
//			LatentActionManager.AddNewAction(Obj, uuid, new FObjectDelayAction<C, Ret, Args...>(Duration, Obj, pf, args...));
//		}
//		else
//		{
//			ObjectDelayAction->SetDuration(Duration);
//		}
//	}
//}

template<class C, typename... Args>
void UTryDelayBPLibrary::DelayMemberFunction(C* Obj, int32 uuid, float Duration, bool bRetriggerable, void(C::* pf)(Args...), Args... args)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(Obj, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();

		FObjectDelayAction<C, Args...>* ObjectDelayAction = LatentActionManager.FindExistingAction<FObjectDelayAction<C, Args...>>(Obj, uuid);
		if (ObjectDelayAction == nullptr)
		{
			LatentActionManager.AddNewAction(Obj, uuid, new FObjectDelayAction<C, Args...>(Duration, Obj, pf, args...));
		}
		else
		{
			ObjectDelayAction->SetDuration(Duration);
		}
	}
}

template<class C, typename... Args>
void UTryDelayBPLibrary::DelayRawFunction(UWorld* World, C* Obj, int32 uuid, float Duration, bool bRetriggerable, void(C::* pf)(Args...), Args... args)
{
	if (World)
	{
		UE_LOG(LogTemp, Warning, TEXT("%d"), World->IsGameWorld());
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();

		static int32 NewId = 0;
		if (uuid == -1)
		{
			uuid = ++NewId;
		}

		FRawDelayAction<C, Args...>* ObjectDelayAction = LatentActionManager.FindExistingAction<FRawDelayAction<C, Args...>>(World, uuid);
		if (ObjectDelayAction == nullptr)
		{
			LatentActionManager.AddNewAction(World, uuid, new FRawDelayAction<C, Args...>(Duration, Obj, pf, args...));
		}
		else
		{
			if (bRetriggerable)
			{
				ObjectDelayAction->SetDuration(Duration);
			}
		}
		NewId = uuid;
	}
}
