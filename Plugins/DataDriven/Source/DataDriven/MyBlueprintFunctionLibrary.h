// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DataTable/MyDataTable.h"
#include "MyBlueprintFunctionLibrary.generated.h"

class UMyPrimaryDataAsset;
class AMyWorldSettings;
/**
 * 
 */
UCLASS()
class DATADRIVEN_API UMyBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable)
	static TArray<FMyDataTable> LoadMyDataTable();

	UFUNCTION(BlueprintCallable)
	static AMyWorldSettings* GetMyWorldSettings(AActor* InActor);

	UFUNCTION(BlueprintCallable)
	static TArray<UObject*> FindObjectsByClass(TArray<UClass*> InClass, bool bRecursivePaths = false);

	UFUNCTION(BlueprintCallable)
	static TArray<UObject*> FindObjectsByClassAndPaths(TArray<UClass*> InClass, TArray<FString> PackagePaths, bool bRecursivePaths = false);
};
