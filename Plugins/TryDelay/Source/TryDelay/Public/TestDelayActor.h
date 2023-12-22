// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestDelayActor.generated.h"

class FRawDelayTest
{
public:
	void Print(UWorld* World)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Black, FString::Printf(TEXT("%f"), World->GetRealTimeSeconds()));
	}
	void PrintVlaue(UWorld* World, int32 Value)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Black, FString::Printf(TEXT("%f"), World->GetRealTimeSeconds()));
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("%d"), Value));
	}
};

UCLASS()
class TRYDELAY_API ATestDelayActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATestDelayActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void Print();

	void PrintValue(int a);

	FRawDelayTest* RawDelayTest = nullptr;
};
