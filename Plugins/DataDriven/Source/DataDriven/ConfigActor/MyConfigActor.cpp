// Fill out your copyright notice in the Description page of Project Settings.


#include "MyConfigActor.h"

// Sets default values
AMyConfigActor::AMyConfigActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMyConfigActor::BeginPlay()
{
	Super::BeginPlay();
	
	FString InPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("Test/Test.ini"));
	//SaveConfig(CPF_Config, *InPath);
	GConfig->LoadFile(InPath);
	LoadConfig(nullptr, *InPath);
}


