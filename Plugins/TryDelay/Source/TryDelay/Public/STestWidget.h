// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class TRYDELAY_API STestWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STestWidget)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

protected:
	FReply ClickButton();

	void PrintValue(UWorld* World, float InValue);

private:
	UWorld* World = nullptr;
};
