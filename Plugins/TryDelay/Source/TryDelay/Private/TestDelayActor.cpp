// Fill out your copyright notice in the Description page of Project Settings.


#include "TestDelayActor.h"
#include "TryDelayBPLibrary.h"
#include "Templates/Function.h"
#include "TestRaw.h"
#include "Components/ActorComponent.h"
#include "UObject/ObjectPtr.h"
#include "Containers/Set.h"
#include "CommonUtilBPLibrary.h"

// Sets default values
ATestDelayActor::ATestDelayActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root1"));

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh2"));
	StaticMeshComponent->SetupAttachment(RootComponent);
}

IMPLEMENT_GET_PRIVATE_VAR(FRawDelayTest, b, float);
IMPLEMENT_GET_PRIVATE_VAR(FRawDelayTest, c, double);

// Called when the game starts or when spawned
void ATestDelayActor::BeginPlay()
{
	Super::BeginPlay();
	
	/* 模板部分功能测试
	UE_LOG(LogTemp, Warning, TEXT("%d, %d"), Tmp::integral_constant<bool, false>::value, Tmp::true_type::value);
	UE_LOG(LogTemp, Warning, TEXT("%d, %d, %d"), Tmp::Is_Conversion_To<float, double>::value, Tmp::Is_Conversion_To<ATestDelayActor*, AActor*>::value, Tmp::Has_DefaultConstructor_v<ATestDelayActor>);
	*/

	/* 私有变量获取
	double* PrivateData = GET_PRIVATE(FRawDelayTest, RawDelayTest, c);
	float* ProtectData = GET_PRIVATE(FRawDelayTest, RawDelayTest, b);

	UE_LOG(LogTemp, Warning, TEXT("b:%f, c:%f"), *ProtectData, *PrivateData);
	*ProtectData = 50.1f;
	*PrivateData = 77.7;
	UE_LOG(LogTemp, Warning, TEXT("b:%f, c:%f"), *ProtectData, *PrivateData);*/

	/* 反射调用UFUNCTION函数
	TTuple<int32, bool> Ret2;
	UCommonUtilBPLibrary::CallFunction(this, TEXT("UFunctionTest"), Ret2, 200, 2);
	UE_LOG(LogTemp, Warning, TEXT("%d, %d"), get<0>(Ret2), get<1>(Ret2));

	TTuple<int32, int32> Params(100, 50);
	UCommonUtilBPLibrary::CallFunction(this, TEXT("UFunctionTest"), Ret2, Params);
	UE_LOG(LogTemp, Warning, TEXT("%d, %d"), get<0>(Ret2), get<1>(Ret2));*/

	TTuple<int32, int32> Ret2;
	TTuple<int32, int32> Params(100, 50);
	UCommonUtilBPLibrary::CallFunction(this, TEXT("UFunctionTest2"), Ret2, Params);
	UE_LOG(LogTemp, Warning, TEXT("%d, %d"), get<0>(Ret2), get<1>(Ret2));

	/*UTryDelayBPLibrary::DelayFunctionName(this, TEXT("Print"), 2.f);
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Black, FString::Printf(TEXT("%f"), GetWorld()->GetRealTimeSeconds()));*/

	/*auto Lambda1 = [this]()
	{
		Print();
	};
	UTryDelayBPLibrary::DelayLambda(-1, 1.f, Lambda1, 10);*/
	/*auto Lambda2 = [this](int a)
	{
		PrintValue(a);
		return true;
	};
	UTryDelayBPLibrary::DelayLambda(-1, 1.f, true, Lambda2, 20);*/

	RawDelayTest = new FRawDelayTest(this);

	//UTryDelayBPLibrary::DelayMemberFunction(this, -1, 1.f, true, &ATestDelayActor::Print);
	UTryDelayBPLibrary::DelayMemberFunction(-1, 1.5f, FDelayDelegate::CreateUObject(this, &ATestDelayActor::PrintRet, 10));

	
	UTryDelayBPLibrary::DelayRawFunction(RawDelayTest, -1, 1.f, false, &FRawDelayTest::Print, GetWorld());
	UTryDelayBPLibrary::DelayRawFunction(-1, 2.0f, FDelayDelegate::CreateRaw(RawDelayTest, &FRawDelayTest::PrintVlaue, GetWorld(), 50));
	UTryDelayBPLibrary::DelayRawFunctionForNextTick(FDelayDelegate::CreateRaw(RawDelayTest, &FRawDelayTest::PrintVlaue, GetWorld(), 50));

	UTryDelayBPLibrary::ExecuteOnTick([](float DeltaTime, UWorld* World)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("%f"), World->GetRealTimeSeconds()));
			return false;
		}, GetWorld()
	);
	//FRawTest* RawTest = new FRawTest();
}

// Called every frame
void ATestDelayActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ATestDelayActor::Print()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, FString::Printf(TEXT("%f"), GetWorld()->GetRealTimeSeconds()));
	UE_LOG(LogTemp, Warning, TEXT("Delay:%f"), GetWorld()->GetRealTimeSeconds());

	return true;
}
bool ATestDelayActor::PrintRet(int32 aa)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, FString::Printf(TEXT("%f"), GetWorld()->GetRealTimeSeconds()));
	UE_LOG(LogTemp, Warning, TEXT("Delay:%f, %d"), GetWorld()->GetRealTimeSeconds(), aa);

	return true;
}

bool ATestDelayActor::UFunctionTest(int32& aa, int32 bb)
{
	aa = aa + bb;
	return true;
}

void ATestDelayActor::PrintValue(int aa)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, FString::Printf(TEXT("%d, %f"), aa, GetWorld()->GetRealTimeSeconds()));
	UE_LOG(LogTemp, Warning, TEXT("Delay:%d,%f"), aa, GetWorld()->GetRealTimeSeconds());
}

