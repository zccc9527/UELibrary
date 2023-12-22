// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MyPrimaryDataAsset.generated.h"

/**
 * ͨ�������ʲ��ķ�ʽ��������
 */
UCLASS()
class DATADRIVEN_API UMyPrimaryDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name = TEXT("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UTexture2D* Icon = nullptr;
};
