// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../CDO/CDOStruct.h"
#include "MyConfigActor.generated.h"

/*
* 可以直接将有config修饰的属性写入到ini配置文件中,通过LoadConfig和SaveConfig接口实现
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