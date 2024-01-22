// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "DelayAction.h"
#include "Templates/Function.h"
#include "CommonUtilBPLibrary.h"
#include "TryDelayBPLibrary.generated.h"

/*
* 延迟TFunction表达式
*/
class FTFunctionDelayAction : public FPendingLatentAction
{
public:
	FTFunctionDelayAction(float InDuration, TFunction<void()> InTriggerFunc) : Duration(InDuration), TriggerFunc(InTriggerFunc){}

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
* 延迟Lambda表达式
*/
template<typename TLambda, typename...Args>
class FLambdaDelayAction : public FPendingLatentAction
{
public:
	FLambdaDelayAction(float InDuration, TLambda InTriggerFunc, Args...args) : Duration(InDuration), TriggerFunc(InTriggerFunc), m_payload(std::forward<Args>(args)...) {}

	void SetDuration(float InDuration) { Duration = InDuration; }
private:
	float Duration = -1.0f;
	TLambda TriggerFunc;
	std::tuple<Args...> m_payload;

	template <std::size_t... Index>
	struct Indices {};

	//生成0到N-1的索引
	template <std::size_t N, std::size_t... Index>
	struct build_inds
	{
		typedef typename build_inds<N - 1, N - 1, Index...>::type type;
	};
	template <std::size_t... Index>
	struct build_inds<0, Index...>
	{
		typedef Indices<Index...> type;
	};

	template<std::size_t... Index>
	void Execute(Indices<Index...> Ind)
	{
		TriggerFunc(std::get<Index>(m_payload)...);
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		Duration -= Response.ElapsedTime();
		bool Ok = Duration <= 0.f;
		if (Ok)
		{
			Execute(build_inds<sizeof...(Args)>::type());
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
class FObjectDelayAction : public FPendingLatentAction
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

public:
	/**
	* 根据函数名字延迟调用
	* @param CallbackTarget			需要延迟调用函数的对象
	* @param uuid					标识符,为-1时自动增加
	* @param ExecutionFunction		需要延迟调用的函数名字
	* @param Duration				延迟调用的时间
	* @param bRetriggerable			是否能够重置时间
	*/
	UFUNCTION(BlueprintCallable, Category = "TryDelay")
	static void DelayFunctionName(UObject* CallbackTarget, int32 uuid, FName ExecutionFunction, float Duration, bool bRetriggerable = true);

	/**
	* 延迟调用Lambda表达式/无参数,无返回值
	* @param CallbackTarget			需要延迟调用函数的对象
	* @param uuid					标识符,为-1时自动增加
	* @param ExecutionFunction		需要延迟调用的函数名字
	* @param Duration				延迟调用的时间
	* @param bRetriggerable			是否能够重置时间
	*/
	static void DelayLambda(int32 uuid, float Duration, TFunction<void()> InTriggerFunc, bool bRetriggerable = true);

	/**
	* 延迟调用Lambda表达式/有参数,无返回值
	* @param CallbackTarget			需要延迟调用函数的对象
	* @param uuid					标识符,为-1时自动增加
	* @param ExecutionFunction		需要延迟调用的函数名字
	* @param Duration				延迟调用的时间
	* @param bRetriggerable			是否能够重置时间
	*/
	template<typename TLambda, typename...Args>
	static void DelayLambda_Params(int32 uuid, float Duration, TLambda Lambda, Args...args)
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
		if (World == nullptr) return;

		static int32 NewId = 0;
		if (uuid == -1)
		{
			uuid = ++NewId;
		}

		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FLambdaDelayAction<TLambda, Args...>* LambdaDelayAction = LatentActionManager.FindExistingAction<FLambdaDelayAction<TLambda, Args...>>(World, uuid);
		if (LambdaDelayAction == nullptr)
		{
			LatentActionManager.AddNewAction(World, uuid, new FLambdaDelayAction<TLambda, Args...>(Duration, Lambda, args...));
		}
		else
		{
			LambdaDelayAction->SetDuration(Duration);
		}
		NewId = uuid;
	}

	/**
	* 根据函数名字延迟调用
	* @param CallbackTarget			需要延迟调用函数的对象
	* @param uuid					标识符,为-1时自动增加
	* @param ExecutionFunction		需要延迟调用的函数名字
	* @param Duration				延迟调用的时间
	* @param bRetriggerable			是否能够重置时间
	*/
	template<class C, typename... Args>
	static void DelayMemberFunction(UObject* Obj, int32 uuid, float Duration, bool bRetriggerable, void(C::* pf)(Args...), Args... args)
	{
		UWorld* World = Obj->GetWorld();
		if (World == nullptr)
			return;

		static int32 NewId = 0;
		if (uuid == -1)
		{
			uuid = ++NewId;
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
		NewId = uuid;
	}

	/**
	* 延迟调用原生C++类的成员函数
	* @param Obj				需要调用的成员函数的类对象
	* @param uuid				标识符,为-1时自动增加
	* @param Duration			延迟调用的时间
	* @param bRetriggerable		是否能够重置时间
	* @param pf					类成员函数指针
	* @param args				需要调用的函数参数
	*/
	template<class C, typename... Args>
	static void DelayRawFunction(C* Obj, int32 uuid, float Duration, bool bRetriggerable, void(C::* pf)(Args...), Args... args)
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
		if (World == nullptr) return;

		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();

		static int32 NewId = 0;
		if (uuid == -1)
		{
			uuid = ++NewId;
		}

		FRawDelayAction<C, Args...>* ObjectDelayAction = LatentActionManager.FindExistingAction<FRawDelayAction<C, Args...>>(World, uuid);
		if (ObjectDelayAction == nullptr)
		{
			LatentActionManager.AddNewAction(World, uuid, new FRawDelayAction<C, Args...>(Duration, Obj, pf, args...));
		}
		else
		{
			if (bRetriggerable)
			{
				ObjectDelayAction->SetDuration(Duration);
			}
		}
		NewId = uuid;
	}

};
