// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "DelayAction.h"
#include "Templates/Function.h"
#include "CommonUtilBPLibrary.h"
#include "TryDelayBPLibrary.generated.h"

/*
* �ӳ�TFunction���ʽ
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
* �ӳ�Lambda���ʽ
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

	//����0��N-1������
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
* �ӳ�ԭ�����Ա����
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
* �ӳ�UObject���Ա����
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
	* ���ݺ��������ӳٵ���
	* @param CallbackTarget			��Ҫ�ӳٵ��ú����Ķ���
	* @param uuid					��ʶ��,Ϊ-1ʱ�Զ�����
	* @param ExecutionFunction		��Ҫ�ӳٵ��õĺ�������
	* @param Duration				�ӳٵ��õ�ʱ��
	* @param bRetriggerable			�Ƿ��ܹ�����ʱ��
	*/
	UFUNCTION(BlueprintCallable, Category = "TryDelay")
	static void DelayFunctionName(UObject* CallbackTarget, int32 uuid, FName ExecutionFunction, float Duration, bool bRetriggerable = true);

	/**
	* �ӳٵ���Lambda���ʽ/�޲���,�޷���ֵ
	* @param CallbackTarget			��Ҫ�ӳٵ��ú����Ķ���
	* @param uuid					��ʶ��,Ϊ-1ʱ�Զ�����
	* @param ExecutionFunction		��Ҫ�ӳٵ��õĺ�������
	* @param Duration				�ӳٵ��õ�ʱ��
	* @param bRetriggerable			�Ƿ��ܹ�����ʱ��
	*/
	static void DelayLambda(int32 uuid, float Duration, TFunction<void()> InTriggerFunc, bool bRetriggerable = true);

	/**
	* �ӳٵ���Lambda���ʽ/�в���,�޷���ֵ
	* @param CallbackTarget			��Ҫ�ӳٵ��ú����Ķ���
	* @param uuid					��ʶ��,Ϊ-1ʱ�Զ�����
	* @param ExecutionFunction		��Ҫ�ӳٵ��õĺ�������
	* @param Duration				�ӳٵ��õ�ʱ��
	* @param bRetriggerable			�Ƿ��ܹ�����ʱ��
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
	* ���ݺ��������ӳٵ���
	* @param CallbackTarget			��Ҫ�ӳٵ��ú����Ķ���
	* @param uuid					��ʶ��,Ϊ-1ʱ�Զ�����
	* @param ExecutionFunction		��Ҫ�ӳٵ��õĺ�������
	* @param Duration				�ӳٵ��õ�ʱ��
	* @param bRetriggerable			�Ƿ��ܹ�����ʱ��
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
	* �ӳٵ���ԭ��C++��ĳ�Ա����
	* @param Obj				��Ҫ���õĳ�Ա�����������
	* @param uuid				��ʶ��,Ϊ-1ʱ�Զ�����
	* @param Duration			�ӳٵ��õ�ʱ��
	* @param bRetriggerable		�Ƿ��ܹ�����ʱ��
	* @param pf					���Ա����ָ��
	* @param args				��Ҫ���õĺ�������
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
