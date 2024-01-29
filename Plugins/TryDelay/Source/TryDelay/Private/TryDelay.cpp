// Copyright Epic Games, Inc. All Rights Reserved.

#include "TryDelay.h"
#include "TryDelayBPLibrary.h"

#define LOCTEXT_NAMESPACE "FTryDelayModule"

void FTryDelayModule::StartupModule()
{
	FWorldDelegates::OnPostWorldInitialization.AddRaw(this, &FTryDelayModule::OnPostWorldInit);
}

void FTryDelayModule::ShutdownModule()
{
	
}

void FTryDelayModule::OnPostWorldInit(UWorld* InWorld, const UWorld::InitializationValues)
{
	auto Test = []()
	{
		bool bRemove = false;
		static int32 Count = 0;
		UE_LOG(LogTemp, Warning, TEXT("123"));
		Count++;
		bRemove = Count >= 5 ? true : false;

		return bRemove;
	};
	//UTryDelayBPLibrary::DelayLambda(-1, 1.0f, false, Test);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FTryDelayModule, TryDelay)