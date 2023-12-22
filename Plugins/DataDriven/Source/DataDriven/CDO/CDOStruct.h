// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CDOStruct.generated.h"

/**
 * ͨ���ṹ������洢����
 */
USTRUCT(BlueprintType)
struct FCDOStruct 
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name = TEXT("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UTexture2D* Icon = nullptr;
};
