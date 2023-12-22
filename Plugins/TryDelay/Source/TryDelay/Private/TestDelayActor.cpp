// Fill out your copyright notice in the Description page of Project Settings.


#include "TestDelayActor.h"
#include "TryDelayBPLibrary.h"
#include "Templates/Function.h"
#include "TestRaw.h"
#include "CommonUtilBPLibrary.h"

// Sets default values
ATestDelayActor::ATestDelayActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATestDelayActor::BeginPlay()
{
	Super::BeginPlay();
	
	/*UTryDelayBPLibrary::DelayFunctionName(this, TEXT("Print"), 2.f);
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Black, FString::Printf(TEXT("%f"), GetWorld()->GetRealTimeSeconds()));*/

	/*int32 a = 10;
	int32 b = 20;
	auto Lambda1 = [this]()
	{
		Print();
	};
	UTryDelayBPLibrary::DelayLambda(this, 1, 2.f, Lambda1);
	auto Lambda2 = [Lambda1, this]()
	{
		Print();
		UTryDelayBPLibrary::DelayLambda(this, 1, 1.5f, Lambda1);
	};
	UTryDelayBPLibrary::DelayLambda(this, 2, 1.5f, Lambda2);*/

	//UTryDelayBPLibrary::DelayMemberFunction(this, 1, 2.f, &ATestDelayActor::Print);
	//UTryDelayBPLibrary::DelayMemberFunction(this, 2, 3.f, &ATestDelayActor::PrintValue, 100);
	//UTryDelayBPLibrary::DelayMemberFunction(this, 3, 5.f, &ATestDelayActor::Print);

	//RawDelayTest = new FRawDelayTest();
	////UTryDelayBPLibrary::DelayRawFunction(RawDelayTest, 1, 5.f, &FRawDelayTest::Print, GetWorld());
	//UTryDelayBPLibrary::DelayRawFunction(RawDelayTest, 1, 4.f, &FRawDelayTest::PrintVlaue, GetWorld(), 50);

	//FRawTest* RawTest = new FRawTest();

	/*UE_LOG(LogTemp, Warning, TEXT("%d, %d"), Tmp::integral_constant<bool, false>::value, Tmp::true_type::value);
	UE_LOG(LogTemp, Warning, TEXT("%d, %d, %d"), Tmp::Is_Conversion_To<float, double>::value, Tmp::Is_Conversion_To<ATestDelayActor*, AActor*>::value, Tmp::Has_DefaultConstructor_v<ATestDelayActor>);*/

}

// Called every frame
void ATestDelayActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATestDelayActor::Print()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("%f"), GetWorld()->GetRealTimeSeconds()));
}

void ATestDelayActor::PrintValue(int a)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("%f"), GetWorld()->GetRealTimeSeconds()));
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Black, FString::Printf(TEXT("%d"), a));
}

