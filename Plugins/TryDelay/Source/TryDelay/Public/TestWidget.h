// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "TestWidget.generated.h"

/**
 * 
 */
UCLASS()
class TRYDELAY_API UTestWidget : public UWidget
{
	GENERATED_BODY()
public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
};
