// Fill out your copyright notice in the Description page of Project Settings.


#include "STestWidget.h"
#include "SlateOptMacros.h"
#include "TryDelayBPLibrary.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void STestWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SButton).Text(FText::FromString(TEXT("点击"))).OnClicked(this, &STestWidget::ClickButton)
	];
	if (GWorld->GetWorld() && GWorld->GetWorld()->IsGameWorld())
	{
		World = GWorld->GetWorld();
		UTryDelayBPLibrary::DelayRawFunction(World, this, -1, 5.f, true, &STestWidget::PrintValue, World, 110.f);
	}
}

FReply STestWidget::ClickButton()
{
	if (World == nullptr) return FReply::Unhandled();

	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("%f"), World->GetRealTimeSeconds()));
	auto TestLam = [this]()
	{
		UTryDelayBPLibrary::DelayRawFunction(World, this, -1, 5.f, true, &STestWidget::PrintValue, World, 220.f);
	};
	UTryDelayBPLibrary::DelayLambda(World, -1, 1.f, TestLam);

	return FReply::Unhandled();
}

void STestWidget::PrintValue(UWorld* InWorld, float InValue)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("%f"), InWorld->GetRealTimeSeconds()));
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Black, FString::Printf(TEXT("%f"), InValue));
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION