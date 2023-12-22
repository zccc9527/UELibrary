// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "../CDO/CDOStruct.h"
#include "MyWorldSettings.generated.h"

/**
 * ÿ���ؿ���ֻ��һ��WorldSettings,���Խ����ݷ��뵽WorldSettings�б���
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
