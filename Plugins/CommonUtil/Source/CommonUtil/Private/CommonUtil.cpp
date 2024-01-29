// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonUtil.h"
#include "CommonUtilBPLibrary.h"
#include "Widgets/SWidget.h"
#include "CommonFunctionalClass.h"

#define LOCTEXT_NAMESPACE "FCommonUtilModule"

DEFINE_HAS_FUNC_MEM(func);
DEFINE_HAS_TYPE_MEM(Type);
DEFINE_HAS_VALUE_MEM(a);

PRAGMA_DISABLE_OPTIMIZATION
void FCommonUtilModule::StartupModule()
{
	TFunction<void(float)> GetWidget = [](float Value)
	{
		static bool bFirst = true;
		if (GWorld && GWorld->GetWorld() && bFirst)
		{
			UE_LOG(LogTemp, Warning, TEXT("Begin"));
			bFirst = false;
			FTimerHandle TimeHandle;

			GWorld->GetWorld()->GetTimerManager().SetTimer(TimeHandle, []()
				{
					UE_LOG(LogTemp, Warning, TEXT("123"));
				}, 5.f, true);
		}
	};
	if (FSlateApplication::IsInitialized())
	{
		//FSlateApplication::Get().OnPostTick().AddLambda(GetWidget);
	}

	/*OnUObjectCreateListener = new FOnUObjectCreateListener();*/
}
PRAGMA_ENABLE_OPTIMIZATION
void FCommonUtilModule::ShutdownModule()
{
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCommonUtilModule, CommonUtil)
