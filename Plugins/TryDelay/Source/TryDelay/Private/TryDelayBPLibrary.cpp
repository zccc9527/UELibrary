// Copyright Epic Games, Inc. All Rights Reserved.

#include "TryDelayBPLibrary.h"
#include "DelayAction.h"
#include "UObject/NoExportTypes.h"
#include "UObject/SavePackage.h"

int32 UTryDelayBPLibrary::DelayFunctionName(UObject* CallbackTarget, int32 uuid, FName ExecutionFunction, float Duration, bool bRetriggerable/* = false*/)
{
	if (UWorld* World = CallbackTarget->GetWorld())
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();

		if (uuid == -1)
		{
			uuid = UCommonUtilBPLibrary::GenerateUniqueID();
		}

		FUFunctionDelayAction<>* DelayAction = LatentActionManager.FindExistingAction<FUFunctionDelayAction<>>(CallbackTarget, uuid);
		if (DelayAction == nullptr)
		{
			LatentActionManager.AddNewAction(CallbackTarget, uuid, new FUFunctionDelayAction(CallbackTarget, Duration, ExecutionFunction));
		}
		else
		{
			if (bRetriggerable)
			{
				DelayAction->SetDuration(Duration);
			}
		}
		return uuid;
	}
	return -1;
}

void UTryDelayBPLibrary::DelayFunctionNameForNextTick(UObject* CallbackTarget, FName ExecutionFunction)
{
	UTryDelayBPLibrary::DelayFunctionName(CallbackTarget, -1, ExecutionFunction, 0.f);
}
