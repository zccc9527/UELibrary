// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "DelayAction.h"
#include "Templates/Function.h"
#include "TryDelayBPLibrary.generated.h"

/*
* 延迟Lambda表达式
*/
class FLambdaDelayAction : public FPendingLatentAction
{
public:
	FLambdaDelayAction(float InDuration, TFunction<void()> InTriggerFunc) : Duration(InDuration), TriggerFunc(InTriggerFunc){}

	void SetDuration(float InDuration) { Duration = InDuration; }
private:
	float Duration;
	TFunction<void()> TriggerFunc;
	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		Duration -= Response.ElapsedTime();
		bool Ok = Duration <= 0.f;
		if (Ok)
		{
			TriggerFunc();
		}
		Response.DoneIf(Ok);
	}
};

/*
* 延迟原生类成员函数
*/
template<class C, typename... Args>
class FRawDelayAction : public FPendingLatentAction
{
public:
	using FuncPtr = void(C::*)(Args...);
	FRawDelayAction(float InDuration, C* p, FuncPtr pf, Args... args)
	{
		Duration = InDuration;
		TriggerFunc.BindRaw(p, pf, args...);
	};

	void SetDuration(float InDuration) { Duration = InDuration; }
private:
	float Duration;
	TDelegate<void(void)> TriggerFunc;
	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		Duration -= Response.ElapsedTime();
		bool Ok = Duration <= 0.f;
		if (Ok)
		{
			TriggerFunc.ExecuteIfBound();
		}
		Response.DoneIf(Ok);
	}
};

/*
* 延迟UObject类成员函数
*/
template<class C, typename... Args>
class FObjectDelayAction/*<C, void, Args...>*/ : public FPendingLatentAction
{
public:
	using FuncPtr = void(C::*)(Args...);
	FObjectDelayAction(float InDuration, C* p, FuncPtr pf, Args... args)
	{
		Duration = InDuration;
		TriggerFunc.BindUObject(p, pf, args...);
	};

	void SetDuration(float InDuration) { Duration = InDuration; }
private:
	float Duration;
	TDelegate<void(void)> TriggerFunc;
	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		Duration -= Response.ElapsedTime();
		bool Ok = Duration <= 0.f;
		if (Ok)
		{
			TriggerFunc.ExecuteIfBound();
		}
		Response.DoneIf(Ok);
	}
};

UCLASS()
class TRYDELAY_API UTryDelayBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/*UFUNCTION(BlueprintCallable, meta = (DisplayName = "Execute Sample function", Keywords = "TryDelay sample test testing"), Category = "TryDelayTesting")
	static float TryDelaySampleFunction(float Param);*/

public:
	UFUNCTION(BlueprintCallable, Category = "TryDelay")
	static void DelayFunctionName(UObject* CallbackTarget, FName ExecutionFunction, float Duration, bool bRetriggerable = true);

	//UFUNCTION(BlueprintCallable, Category = "TryDelay")
	static void DelayLambda(UObject* Obj, int32 uuid, float Duration, TFunction<void()> InTriggerFunc, bool bRetriggerable = true);

	template<class C, typename... Args>
	static void DelayMemberFunction(C* Obj, int32 uuid, float Duration, bool bRetriggerable,  void(C::* pf)(Args...), Args... args);

	template<class C, typename... Args>
	static void DelayRawFunction(UWorld* World, C* Obj, int32 uuid, float Duration, bool bRetriggerable, void(C::* pf)(Args...), Args... args);
};
