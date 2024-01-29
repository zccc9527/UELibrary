// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "DelayAction.h"
#include "Templates/Function.h"
#include "CommonUtilBPLibrary.h"
#include "DelayManager.h"
#include "TryDelayBPLibrary.generated.h"

DECLARE_DELEGATE_RetVal(bool, FDelayDelegate);
DECLARE_DYNAMIC_DELEGATE_RetVal(bool, FDelayDynamicDelegate);

UCLASS()
class TRYDELAY_API UTryDelayBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	* 根据函数名字延迟调用
	* @param CallbackTarget			需要延迟调用函数的对象
	* @param uuid					标识符,为-1时自动生成唯一标识符
	* @param ExecutionFunction		需要延迟调用的函数名字
	* @param Duration				延迟调用的时间
	* @param bRetriggerable			是否能够重置时间
	* @return						返回标识符,可根据标识符在延迟时间前重置时间
	*/
	UFUNCTION(BlueprintCallable, Category = "TryDelay")
	static int32 DelayFunctionName(UObject* CallbackTarget, int32 uuid, FName ExecutionFunction, float Duration, bool bRetriggerable = false);

	UFUNCTION(BlueprintCallable, Category = "TryDelay")
	static void DelayFunctionNameForNextTick(UObject* CallbackTarget, FName ExecutionFunction);

	template<typename TLambda, typename... Args>
	static void ExecuteOnTick(TLambda InFunc, Args...args);
	/**
	* 延迟调用Lambda表达式
	* @param uuid					标识符,为-1时自动生成唯一标识符
	* @param Duration				延迟调用的时间
	* @param bRetriggerable			是否能够重置时间
	* @param InTriggerFunc			Lambda表达式
	* @param args					重载参数
	* @return						返回标识符,可根据标识符在延迟时间前重置时间
	*/
	template<typename TLambda, typename...Args>
	static int32 DelayLambda(int32 uuid, float Duration, bool bRetriggerable, TLambda InTriggerFunc, Args...args);

	template<typename TLambda, typename...Args>
	static void DelayLambdaForNextTick(TLambda InTriggerFunc, Args...args);

	/**
	* 延迟调用UObject的类成员函数
	* @param Obj					需要延迟调用函数的对象
	* @param uuid					标识符,为-1时自动生成唯一标识符
	* @param Duration				延迟调用的时间
	* @param bRetriggerable			是否能够重置时间
	* @param pf						延迟调用的UObject成员函数
	* @param args					重载参数
	* @return						返回标识符,可根据标识符在延迟时间前重置时间
	*/
	template<class C, typename... Args>
	static int32 DelayMemberFunction(UObject* Obj, int32 uuid, float Duration, bool bRetriggerable, bool(C::* pf)(Args...), Args... args);

	template<typename... Args>
	static int32 DelayMemberFunction(int32 uuid, float Duration, const FDelayDelegate& InDelegate, bool bRetriggerable = false);

	template<typename... Args>
	static void DelayMemberFunctionForNextTick(const FDelayDelegate& InDelegate);

	/**
	* 延迟调用原生C++类的成员函数
	* @param Obj				需要调用的成员函数的类对象
	* @param uuid				标识符,为-1时自动生成唯一标识符
	* @param Duration			延迟调用的时间
	* @param bRetriggerable		是否能够重置时间
	* @param pf					类成员函数指针
	* @param args				需要调用的函数参数
	* @return					返回标识符,可根据标识符在延迟时间前重置时间
	*/
	template<class C, typename... Args>
	static int32 DelayRawFunction(C* Obj, int32 uuid, float Duration, bool bRetriggerable, bool(C::* pf)(Args...), Args... args);

	template<typename... Args>
	static int32 DelayRawFunction(int32 uuid, float Duration, const FDelayDelegate& InDelegate, bool bRetriggerable = false);

	template<typename... Args>
	static void DelayRawFunctionForNextTick(const FDelayDelegate& InDelegate);
};

template<typename TLambda, typename...Args>
void UTryDelayBPLibrary::ExecuteOnTick(TLambda InFunc, Args...args)
{
	auto FindWorld = [](UWorld* InWorld)->bool
	{
		#if WITH_EDITOR || GIsEditor
		if (InWorld->WorldType == EWorldType::PIE)	return true;
		#else
		if (InWorld->WorldType == EWorldType::Game || InWorld->WorldType == EWorldType::GamePreview) return true;
		#endif
		return false;
	};

	UWorld* World = UCommonUtilBPLibrary::ForEachWorld(FindWorld);
	if (World == nullptr) World = GWorld ? GWorld->GetWorld() : nullptr;
	if (World == nullptr) return;
	if (World)
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();

		uint32 uuid = UCommonUtilBPLibrary::GenerateUniqueID();

		FTickableFunctor<TLambda, Args...>* DelayAction = LatentActionManager.FindExistingAction<FTickableFunctor<TLambda, Args...>>(World, uuid);
		if (DelayAction == nullptr)
		{
			LatentActionManager.AddNewAction(World, uuid, new FTickableFunctor<TLambda, Args...>(InFunc, args...));
		}
	}
}

template<typename TLambda, typename...Args>
int32 UTryDelayBPLibrary::DelayLambda(int32 uuid, float Duration, bool bRetriggerable, TLambda InTriggerFunc, Args...args)
{
	auto FindWorld = [](UWorld* InWorld)->bool
	{
		#if WITH_EDITOR || GIsEditor
		if (InWorld->WorldType == EWorldType::PIE)	return true;
		#else
		if (InWorld->WorldType == EWorldType::Game || InWorld->WorldType == EWorldType::GamePreview) return true;
		#endif
		return false;
	};

	UWorld* World = UCommonUtilBPLibrary::ForEachWorld(FindWorld);
	if (World == nullptr) World = GWorld ? GWorld->GetWorld() : nullptr;
	if (World == nullptr) return -1;

	if (uuid == -1)
	{
		uuid = UCommonUtilBPLibrary::GenerateUniqueID();
	}

	FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
	FLambdaDelayAction<TLambda, Args...>* LambdaDelayAction = LatentActionManager.FindExistingAction<FLambdaDelayAction<TLambda, Args...>>(World, uuid);
	if (LambdaDelayAction == nullptr)
	{
		LatentActionManager.AddNewAction(World, uuid, new FLambdaDelayAction<TLambda, Args...>(Duration, InTriggerFunc, args...));
	}
	else
	{
		if (bRetriggerable)
		{
			LambdaDelayAction->SetDuration(Duration);
		}
	}
	return uuid;
}

template<typename TLambda, typename...Args>
void UTryDelayBPLibrary::DelayLambdaForNextTick(TLambda InTriggerFunc, Args...args)
{
	UTryDelayBPLibrary::DelayLambda(-1, 0.0f, false, InTriggerFunc, args...);
}

template<class C, typename...Args>
int32 UTryDelayBPLibrary::DelayMemberFunction(UObject* Obj, int32 uuid, float Duration, bool bRetriggerable, bool(C::* pf)(Args...), Args... args)
{
	UWorld* World = Obj->GetWorld();
	if (World == nullptr)
		return -1;

	if (uuid == -1)
	{
		uuid = UCommonUtilBPLibrary::GenerateUniqueID();
	}

	FLatentActionManager& LatentActionManager = World->GetLatentActionManager();

	FObjectDelayAction<C, Args...>* ObjectDelayAction = LatentActionManager.FindExistingAction<FObjectDelayAction<C, Args...>>(Obj, uuid);
	if (ObjectDelayAction == nullptr)
	{
		LatentActionManager.AddNewAction(Obj, uuid, new FObjectDelayAction<C, Args...>(Duration, Cast<C>(Obj), pf, args...));
	}
	else
	{
		if (bRetriggerable)
		{
			ObjectDelayAction->SetDuration(Duration);
		}
	}
	return uuid;
}

template<typename...Args>
int32 UTryDelayBPLibrary::DelayMemberFunction(int32 uuid, float Duration, const FDelayDelegate& InDelegate, bool bRetriggerable /*= false*/)
{
	UObject* Obj = InDelegate.GetUObject();
	checkf(Obj, TEXT("No Bound To UObject"));
	if (Obj)
	{
		UWorld* World = Obj->GetWorld();
		if (World == nullptr)
			return -1;

		if (uuid == -1)
		{
			uuid = UCommonUtilBPLibrary::GenerateUniqueID();
		}

		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();

		FObjectDelayAction<UObject, Args...>* ObjectDelayAction = LatentActionManager.FindExistingAction<FObjectDelayAction<UObject, Args...>>(Obj, uuid);
		if (ObjectDelayAction == nullptr)
		{
			LatentActionManager.AddNewAction(Obj, uuid, new FObjectDelayAction<UObject, Args...>(Duration, InDelegate));
		}
		else
		{
			if (bRetriggerable)
			{
				ObjectDelayAction->SetDuration(Duration);
			}
		}
	}
	return uuid;
}

template<typename...Args>
void UTryDelayBPLibrary::DelayMemberFunctionForNextTick(const FDelayDelegate& InDelegate)
{
	UTryDelayBPLibrary::DelayMemberFunction(-1, 0.0f, InDelegate, false);
}

template<class C, typename...Args>
int32 UTryDelayBPLibrary::DelayRawFunction(C* Obj, int32 uuid, float Duration, bool bRetriggerable, bool(C::* pf)(Args...), Args... args)
{
	auto FindWorld = [](UWorld* InWorld)->bool
	{
		#if WITH_EDITOR || GIsEditor
		if (InWorld->WorldType == EWorldType::PIE)	return true;
		#else
		if (InWorld->WorldType == EWorldType::Game || InWorld->WorldType == EWorldType::GamePreview) return true;
		#endif
		return false;
	};

	UWorld* World = UCommonUtilBPLibrary::ForEachWorld(FindWorld);
	if (World == nullptr) World = GWorld->GetWorld();
	if (World == nullptr) return -1;

	FLatentActionManager& LatentActionManager = World->GetLatentActionManager();

	if (uuid == -1)
	{
		uuid = UCommonUtilBPLibrary::GenerateUniqueID();
	}

	FRawDelayAction<Args...>* ObjectDelayAction = LatentActionManager.FindExistingAction<FRawDelayAction<Args...>>(World, uuid);
	if (ObjectDelayAction == nullptr)
	{
		LatentActionManager.AddNewAction(World, uuid, new FRawDelayAction<Args...>(Duration, Obj, pf, args...));
	}
	else
	{
		if (bRetriggerable)
		{
			ObjectDelayAction->SetDuration(Duration);
		}
	}
	return uuid;
}

template<typename...Args>
int32 UTryDelayBPLibrary::DelayRawFunction(int32 uuid, float Duration, const FDelayDelegate& InDelegate, bool bRetriggerable /*= false*/)
{
	auto FindWorld = [](UWorld* InWorld)->bool
	{
		#if WITH_EDITOR || GIsEditor
		if (InWorld->WorldType == EWorldType::PIE)	return true;
		#else
		if (InWorld->WorldType == EWorldType::Game || InWorld->WorldType == EWorldType::GamePreview) return true;
		#endif
		return false;
	};

	UWorld* World = UCommonUtilBPLibrary::ForEachWorld(FindWorld);
	if (World == nullptr) World = GWorld->GetWorld();
	if (World == nullptr) return -1;

	FLatentActionManager& LatentActionManager = World->GetLatentActionManager();

	if (uuid == -1)
	{
		uuid = UCommonUtilBPLibrary::GenerateUniqueID();
	}

	FRawDelayAction<Args...>* ObjectDelayAction = LatentActionManager.FindExistingAction<FRawDelayAction<Args...>>(World, uuid);
	if (ObjectDelayAction == nullptr)
	{
		LatentActionManager.AddNewAction(World, uuid, new FRawDelayAction<Args...>(Duration, InDelegate));
	}
	else
	{
		if (bRetriggerable)
		{
			ObjectDelayAction->SetDuration(Duration);
		}
	}
	return uuid;
}

template<typename...Args>
void UTryDelayBPLibrary::DelayRawFunctionForNextTick(const FDelayDelegate& InDelegate)
{
	UTryDelayBPLibrary::DelayRawFunction(-1, 0.0f, InDelegate, false);
}

