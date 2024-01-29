// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CommonUtilBPLibrary.h"
#include "TestDelayActor.generated.h"

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
	bool Print();

	UFUNCTION()
	bool PrintRet(int32 aa);

	UFUNCTION()
	bool UFunctionTest(int32& aa, int32 bb);

	UFUNCTION(BlueprintImplementableEvent)
	int32 UFunctionTest2(int32& aa, int32 bb);

	void PrintValue(int a);

	void PrintA()
	{
		UE_LOG(LogTemp, Warning, TEXT("%d"), a);
	}

	class FRawDelayTest* RawDelayTest = nullptr;

	UStaticMeshComponent* StaticMeshComponent = nullptr;

protected:
	void PrintAPro()
	{
		UE_LOG(LogTemp, Warning, TEXT("protected:%d"), a);
	}
private:
	void PrintAPri()
	{
		UE_LOG(LogTemp, Warning, TEXT("private:%d"), a);
	}
	void PrintAPri(int b)
	{
		UE_LOG(LogTemp, Warning, TEXT("%d, %d"), a, b);
	}
	int GetA() { return a; }
	int a = 20;
};

IMPLEMENT_GET_PRIVATE_VAR(ATestDelayActor, a, int);

IMPLEMENT_GET_PRIVATE_FUNC(ATestDelayActor, PrintAPri, void);
IMPLEMENT_GET_PRIVATE_FUNC(ATestDelayActor, PrintAPro, void);
IMPLEMENT_GET_PRIVATE_FUNC(ATestDelayActor, GetA, int);

IMPLEMENT_GET_PRIVATE_FUNC_OVERLOAD(ATestDelayActor, PrintAPri, void, _2, int);

UE_DISABLE_OPTIMIZATION
class FRawDelayTest
{
public:
	FRawDelayTest(ATestDelayActor* InActor)
	{
		/*int* a3 = GET_PRIVATE(ATestDelayActor, InActor, a);
		UE_LOG(LogTemp, Error, TEXT("%d"), *a3);
		InActor->PrintA();
		CALL_PRIVATE_FUNC(ATestDelayActor, InActor, PrintAPri);

		CALL_PRIVATE_FUNC_OVERLOAD(ATestDelayActor, InActor, PrintAPri, _2, 66);
		int a2 = CALL_PRIVATE_FUNC(ATestDelayActor, InActor, GetA);
		*a3 = 200;
		UE_LOG(LogTemp, Error, TEXT("%d, %d"), a2, *a3);
		InActor->PrintA();
		CALL_PRIVATE_FUNC(ATestDelayActor, InActor, PrintAPro);*/
	}
	bool Print(UWorld* World)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("%f"), World->GetRealTimeSeconds()));
		
		return true;
	}
	bool PrintVlaue(UWorld* World, int32 Value)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("%f"), World->GetRealTimeSeconds()));
		UE_LOG(LogTemp, Warning, TEXT("Delay:%f"), World->GetRealTimeSeconds())
		//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("%d"), Value));

		return true;
	}
	void PrintVlaueRed(UWorld* World)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("%f"), World->GetRealTimeSeconds()));
		UE_LOG(LogTemp, Error, TEXT("Timer:%f"), World->GetRealTimeSeconds())
			//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("%d"), Value));
	}
	int a = 10;
protected:
	float b = 1.2f;
private:
	double c = 20.1;
};
UE_ENABLE_OPTIMIZATION