// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "../CDO/CDOStruct.h"
#include "MyWorldSettings.generated.h"

/**
 * 每个关卡都只有一个WorldSettings,所以将数据放入到WorldSettings中保存
 */
UCLASS()
class AMyWorldSettings : public AWorldSettings
{
	GENERATED_BODY()
	
public:
	AMyWorldSettings(){}
	~AMyWorldSettings(){}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FCDOStruct> CDOStructs;

};
