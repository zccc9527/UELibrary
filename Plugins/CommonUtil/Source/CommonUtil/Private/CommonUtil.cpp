// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonUtil.h"
#include "CommonUtilBPLibrary.h"
#include "Widgets/SWidget.h"

#define LOCTEXT_NAMESPACE "FCommonUtilModule"

DEFINE_HAS_FUNC_MEM(func);
DEFINE_HAS_TYPE_MEM(Type);
DEFINE_HAS_VALUE_MEM(a);

void FCommonUtilModule::StartupModule()
{
	/*TFunction<void(float)> GetWidget = [](float Value)
	{
		class A
		{
		public:
			using Type = int;
			Type a = 10;
			void func(){}
		};
		
		UE_LOG(LogTemp, Warning, TEXT("%d, %d, %d"), CLASS_HAS_FUNC_MEM(A, func), CLASS_HAS_VALUE_MEM(A, a), CLASS_HAS_TYPE_MEM(A, Type));
	};
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().OnPostTick().AddLambda(GetWidget);
	}*/
}

void FCommonUtilModule::ShutdownModule()
{
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCommonUtilModule, CommonUtil)