// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MyDataTable.generated.h"

/**
 * ���ݱ��ÿһ�е�����,���Լ����Ϊ��������ʲ�
 */
USTRUCT(BlueprintType)
struct FMyDataTable : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	FMyDataTable(){}
	~FMyDataTable(){}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name = TEXT("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UTexture2D* Icon = nullptr;
};