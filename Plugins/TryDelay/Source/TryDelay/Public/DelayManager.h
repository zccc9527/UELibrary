#pragma once

#include "CoreMinimal.h"
#include "DelayAction.h"
#include "Templates/Function.h"
#include "CommonUtilBPLibrary.h"

template<typename TLambda, typename...Args>
bool Is_Bool_Ret(TLambda Lambda, Args...args)
{
	return sizeof(bool) == sizeof(decltype(Lambda(args...)));
}

template<typename TLambda, typename... Args>
class FTickableFunctor : public FPendingLatentAction
{
public:
	FTickableFunctor(TLambda InTriggerFunc, Args...args) : TriggerFunc(InTriggerFunc), Params(Forward<Args>(args)...) 
	{
		checkf(Is_Bool_Ret(TriggerFunc, 1.0, args...), TEXT("Lambda Not Return Bool Or Param Not Have Float"));
	}

protected:
	template<std::size_t... Index>
	bool Execute(float DeltaTime, Tmp::Indices<Index...> Ind)
	{
		return TriggerFunc(DeltaTime, get<Index>(Params)...);
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		bool Ok = Execute(Response.ElapsedTime(), Tmp::build_inds<sizeof...(Args)>::type());
		Response.DoneIf(Ok);
	}
private:
	TLambda TriggerFunc;
	TTuple<Args...> Params;
};

class FPendingLatentActionBase : public FPendingLatentAction
{
public:
	FPendingLatentActionBase(float InDuration) : Duration(InDuration), StartDuration(InDuration) {}
	void SetDuration(float InDuration) { Duration = InDuration; }
	bool IsNextTick() const { return StartDuration < 0.0001f; }
protected:
	float Duration;
	float StartDuration;
};

/*
* 延迟调用UFunction函数
*/
template<typename...Args>
class FUFunctionDelayAction : public FPendingLatentActionBase
{
public:
	FUFunctionDelayAction(UObject* InCallbackTarget, float InDuration, FName InFunctionName, Args... args) : FPendingLatentActionBase(InDuration), CallbackTarget(InCallbackTarget), FunctionName(InFunctionName), Params(Forward<Args>(args)...) {}
private:
	UObject* CallbackTarget;
	FName FunctionName;
	TTuple<Args...> Params;

	template<std::size_t... Index>
	TTuple<Args..., bool> Execute(Tmp::Indices<Index...> Ind)
	{
		return TTuple<Args..., bool>(get<Index>(Params)..., bool());
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		if (CallbackTarget == nullptr) return;

		Duration -= Response.ElapsedTime();
		bool Ok = Duration <= 0.f;
		if (Ok)
		{
			if (UFunction* ExecutionFunction = CallbackTarget->FindFunction(FunctionName))
			{
				TTuple<Args..., bool> p = Execute(Tmp::build_inds<sizeof...(Args)>::type());

				CallbackTarget->ProcessEvent(ExecutionFunction, &p);
				Ok = get<sizeof...(Args)>(p);

				Duration = StartDuration + Duration;
				if (IsNextTick())
				{
					Ok = true;
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Error Function Name: %s"), *FunctionName.ToString());
			}
		}
		Response.DoneIf(Ok);
	}
};

/*
* 延迟TFunction表达式
*/
template<typename TLambda, typename...Args>
class FLambdaDelayAction : public FPendingLatentActionBase
{
public:
	FLambdaDelayAction(float InDuration, TLambda InTriggerFunc, Args...args) : FPendingLatentActionBase(InDuration), TriggerFunc(InTriggerFunc), m_payload(Forward<Args>(args)...)
	{
		checkf(Is_Bool_Ret(TriggerFunc, args...), TEXT("Lambda Not Return Bool"));
	}
private:
	TLambda TriggerFunc;
	TTuple<Args...> m_payload;

	template<std::size_t... Index>
	bool Execute(Tmp::Indices<Index...> Ind)
	{
		return TriggerFunc(get<Index>(m_payload)...);
	}
	
	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		Duration -= Response.ElapsedTime();
		bool Ok = Duration <= 0.f;
		if (Ok)
		{
			Ok = Execute(Tmp::build_inds<sizeof...(Args)>::type());
			
			Duration = StartDuration + Duration;
			if (IsNextTick())
			{
				Ok = true;
			}
		}
		Response.DoneIf(Ok);
	}
};

/*
* 延迟原生类成员函数
*/
template<typename... Args>
class FRawDelayAction : public FPendingLatentActionBase
{
public:
	template<typename C>
	FRawDelayAction(float InDuration, C* p, bool(C::* pf)(Args...) , Args... args) : FPendingLatentActionBase(InDuration)
	{
		TriggerFunc.BindRaw(p, pf, args...);
	};

	FRawDelayAction(float InDuration, const TDelegate<bool()>& InDelegate) : FPendingLatentActionBase(InDuration)
	{
		TriggerFunc = InDelegate;
	};

	void SetDuration(float InDuration) { Duration = InDuration; }
private:
	TDelegate<bool(void)> TriggerFunc;
	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		Duration -= Response.ElapsedTime();
		bool Ok = Duration <= 0.f;
		if (Ok)
		{
			if (TriggerFunc.IsBound())
			{
				Ok = TriggerFunc.Execute();

				Duration = StartDuration + Duration;
				if (IsNextTick())
				{
					Ok = true;
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Nothing To Execute"));
			}
		}
		Response.DoneIf(Ok);
	}
};

/*
* 延迟UObject类成员函数
*/
template<class C, typename... Args>
class FObjectDelayAction : public FPendingLatentActionBase
{
public:
	using FuncPtr = bool(C::*)(Args...);
	FObjectDelayAction(float InDuration, C* p, FuncPtr pf, Args... args) : FPendingLatentActionBase(InDuration)
	{
		TriggerFunc.BindUObject(p, pf, args...);
	};

	FObjectDelayAction(float InDuration, const TDelegate<bool()>& InDelegate) : FPendingLatentActionBase(InDuration)
	{
		TriggerFunc = InDelegate;
	};

	void SetDuration(float InDuration) { Duration = InDuration; }
private:
	TDelegate<bool(void)> TriggerFunc;
	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		Duration -= Response.ElapsedTime();
		bool Ok = Duration <= 0.f;
		if (Ok)
		{
			if (TriggerFunc.IsBound())
			{
				Ok = TriggerFunc.Execute();

				Duration = StartDuration + Duration;
				if (IsNextTick())
				{
					Ok = true;
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Nothing To Execute"));
			}
		}
		Response.DoneIf(Ok);
	}
};
