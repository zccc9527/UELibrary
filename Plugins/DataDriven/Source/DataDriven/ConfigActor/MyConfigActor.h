// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../CDO/CDOStruct.h"
#include "MyConfigActor.generated.h"

/*
* ����ֱ�ӽ���config���ε�����д�뵽ini�����ļ���,ͨ��LoadConfig��SaveConfig�ӿ�ʵ��
*/
UCLASS(config = MyConfigActor)
class DATADRIVEN_API AMyConfigActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyConfigActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite)
	TArray<FCDOStruct> CDOStructs;
};