// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "CommonUtilBPLibrary.generated.h"


namespace Tmp
{
	template <std::size_t... Index>
	struct COMMONUTIL_API Indices {};

	//生成0到N-1的索引
	template <std::size_t N, std::size_t... Index>
	struct COMMONUTIL_API build_inds
	{
		typedef typename build_inds<N - 1, N - 1, Index...>::type type;
	};
	template <std::size_t... Index>
	struct COMMONUTIL_API build_inds<0, Index...>
	{
		typedef Indices<Index...> type;
	};

	template<typename T, T v>
	struct COMMONUTIL_API integral_constant
	{
		static constexpr T value = v;
		using value_type = T;
		using type = integral_constant; //指向本身类型，起到简便使用的作用
		constexpr value_type operator()() const noexcept { return value; } //重载括号运算符
		constexpr operator value_type() const noexcept { return value; } //类型转换，可以将该类型隐式转换为value_type类型使用
	};

	template<bool B>
	using bool_constant = integral_constant<bool, B>;
	using true_type = bool_constant<true>;
	using false_type = bool_constant<false>;

	template<typename T>
	struct COMMONUTIL_API type_identity
	{
		using type = T;
	};

	template<typename... Args>
	using void_t = void;

	template<typename... Args>
	constexpr bool bool_t = true;

	template<typename T>
	struct COMMONUTIL_API is_reference : public false_type {};
	template<typename T>
	struct COMMONUTIL_API is_reference<T&> : public true_type {};
	template<typename T>
	struct COMMONUTIL_API is_reference<T&&> : public true_type {};
	template<typename T>
	bool is_reference_v = is_reference<T>::value;

	template<typename T>
	struct COMMONUTIL_API is_pointer : public false_type {};
	template<typename T>
	struct COMMONUTIL_API is_pointer<T*> : public true_type {};
	template<typename T>
	bool is_pointer_v = is_pointer<T>::value;

	template<typename T>
	struct COMMONUTIL_API remove_reference : public type_identity<T> {};
	template<typename T>
	struct COMMONUTIL_API remove_reference<T&> : public type_identity<T> {};
	template<typename T>
	struct COMMONUTIL_API remove_reference<T&&> : public type_identity<T> {};
	template<typename T>
	using remove_reference_t = typename remove_reference<T>::type;

	template<typename T, typename U>
	struct COMMONUTIL_API is_same : public false_type {};
	template<typename T>
	struct COMMONUTIL_API is_same<T, T> : public true_type {};
	template<typename T, typename U>
	constexpr bool is_same_v = is_same<T, U>::value;

	template<typename T, typename F, typename... Args>
	struct COMMONUTIL_API is_any_of : public bool_constant<is_any_of<T, F>::value || is_any_of<T, Args...>::value> {};
	template<typename T, typename F>
	struct COMMONUTIL_API is_any_of<T, F> : public is_same<T, F> {};

	template<typename T, typename F, typename... Args>
	constexpr bool is_any_of_v = is_any_of<T, F, Args...>::value;

	//编译期间的if
	template<bool B, typename T, typename F>
	struct COMMONUTIL_API conditional;
	template<typename T, typename F>
	struct COMMONUTIL_API conditional<false, T, F> : public type_identity<F> {};
	template<typename T, typename F>
	struct COMMONUTIL_API conditional<true, T, F> : public type_identity<T> {};
	template<bool B, typename T, typename F>
	using conditional_t = typename conditional<B, T, F>::type;

	//判断变量大小
	template<typename T, std::size_t v>
	struct COMMONUTIL_API is_equal_size : conditional_t<sizeof(T) == v, true_type, false_type> {};
	template<typename T, std::size_t v>
	constexpr bool is_equal_size_v = is_equal_size<T, v>::value;

	template<typename T, typename = void_t<>>
	struct COMMONUTIL_API add_reference
	{
		using lvalue = T;
		using rvalue = T;
	};
	template<typename T>
	struct add_reference<T, void_t<T&>>
	{
		using lvalue = T&;
		using rvalue = T&&;
	};
	template<typename T>
	using add_Lreference = add_reference<T>::lvalue;
	template<typename T>
	using add_Rreference = add_reference<T>::rvalue;

	template<typename T>
	add_Rreference<T> declval();

	//检查模板类型是否是可以拷贝的(只判断赋值运算符能否使用)
	template <typename T, typename = void>
	struct COMMONUTIL_API is_copyable : false_type {};
	template <typename T>
	struct COMMONUTIL_API is_copyable<T, void_t<decltype(declval<T&>() = declval<const T&>())>> : true_type {};
	template <typename T>
	constexpr bool is_copyable_v = is_copyable<T>::value;

	//检查模板类型是否是可以拷贝的(只判断赋值运算符能否使用)
	/*template <typename T, typename = void>
	struct is_copyable_assign : false_type {};
	template <typename T>
	struct is_copyable_assign<T, void_t<decltype(std::declval<T&>(std::declval<const T&>()))>> : true_type {};
	template <typename T>
	constexpr bool is_copyable_assign_v = is_copyable_assign<T>::value;*/

	//判断该类型是否存在默认构造函数
	template<typename T, typename U = void_t<>>
	struct COMMONUTIL_API Has_DefaultConstructor : false_type {};
	template<typename T>
	struct COMMONUTIL_API Has_DefaultConstructor<T, void_t<decltype(T())>> : true_type {};
	template<typename T>
	constexpr bool Has_DefaultConstructor_v = Has_DefaultConstructor<T>::value;

	//判断类型From能否隐式转换为类型To
	template<typename From, typename To>
	struct COMMONUTIL_API Is_Conversion_To
	{
	private:
		static void Test(To) {}

		template<typename T = decltype(Test(declval<From>()))>
		static true_type Conversion(void*);
		static false_type Conversion(...);

	public:
		static constexpr bool value = is_same_v<true_type, decltype(Conversion(nullptr))>;
	};

	//判断一个类型是否是类类型
	template<typename T>
	struct COMMONUTIL_API Is_Calss
	{
	private:
		template<typename U>
		static true_type Test(int U::*);
		static false_type Test(...);
	public:
		static constexpr bool value = is_same_v<true_type, decltype(Test<T>(nullptr))>;
	};

	//判断Base是否是Derived的父类
	template<typename Base, typename Derived>
	struct COMMONUTIL_API Is_A
	{
	private:
		static true_type Test(Base);
		static false_type Test(...);
	public:
		static constexpr bool value = is_same_v<true_type, decltype(Test(declval<Derived>()))>;
	};
}

/**
* 判断某个类中是否存在type类型
*/
#ifndef DEFINE_HAS_TYPE_MEM
#define DEFINE_HAS_TYPE_MEM(type)	\
template<typename T, typename U = Tmp::void_t<>>	\
struct _HAVE_TYPE_MEMBER_##type : Tmp::false_type {};	\
template<typename T>	\
struct _HAVE_TYPE_MEMBER_##type<T, Tmp::void_t<typename T::type>> : Tmp::true_type {};
#endif

/**
* 判断某个类中是否存在value变量
*
*/
#ifndef DEFINE_HAS_VALUE_MEM
#define DEFINE_HAS_VALUE_MEM(value)	\
template<typename T, typename U = Tmp::void_t<>>	\
struct _HAVE_VALUE_MEMBER_##value : Tmp::false_type {};	\
template<typename T>	\
struct _HAVE_VALUE_MEMBER_##value<T, Tmp::void_t<decltype(T::value)>> : Tmp::true_type {};
#endif

/**
* 判断某个类中是否存在func函数
*/
#ifndef DEFINE_HAS_FUNC_MEM
#define DEFINE_HAS_FUNC_MEM(func)	\
template<typename T, typename U = Tmp::void_t<>>	\
struct _HAVE_FUNC_MEMBER_##func : Tmp::false_type {};	\
template<typename T>	\
struct _HAVE_FUNC_MEMBER_##func<T, Tmp::void_t<decltype(Tmp::declval<T>().func())>> : Tmp::true_type {};
#endif


#ifndef CLASS_HAS_TYPE_MEM
#define CLASS_HAS_TYPE_MEM(className, type) _HAVE_TYPE_MEMBER_##type<className>::value
#endif

#ifndef CLASS_HAS_VALUE_MEM
#define CLASS_HAS_VALUE_MEM(className, valueName) _HAVE_VALUE_MEMBER_##valueName<className>::value
#endif

#ifndef CLASS_HAS_FUNC_MEM
#define CLASS_HAS_FUNC_MEM(className, funcName) _HAVE_FUNC_MEMBER_##funcName<className>::value
#endif

#if 0	/**已舍弃*/
#ifndef GeneratePrivateData
#define GeneratePrivateData(type, member, valuetype)	\
template<auto m>	\
struct Tunnel##type##member;	\
template<typename T, typename U, typename T U::* member> \
struct Tunnel##type##member<member>	\
{\
	friend T* GetPrivateData##type##member(U& u)	\
	{\
		return &(u.*member);	\
	}\
};\
template struct Tunnel##type##member<&type::member>;\
valuetype* GetPrivateData##type##member(type& u);
#endif // !GetPrivateData
#endif

template<typename Accessor, typename Accessor::Member Member> struct COMMONUTIL_API AccessPrivate
{
	friend typename Accessor::Member GetPrivate(Accessor InAccessor)
	{
		return Member;
	}
};

/**----------------获得保护/私有变量----------------*/
#define IMPLEMENT_GET_PRIVATE_VAR(InClass, VarName, VarType) \
struct InClass##VarName##Accessor \
{ \
	using Member = VarType InClass::*; \
	\
	friend Member GetPrivate(InClass##VarName##Accessor); \
}; \
\
template struct AccessPrivate<InClass##VarName##Accessor, &InClass::VarName>;

#define GET_PRIVATE(InClass, InObj, MemberName) &((*InObj).*GetPrivate(InClass##MemberName##Accessor()))
/**----------------------End----------------------*/

/**----------------调用保护/私有函数----------------*/
#define IMPLEMENT_GET_PRIVATE_FUNC(InClass, FuncName, FuncRet, ...)	\
struct InClass##FuncName##Accessor	\
{\
	using Member = FuncRet(InClass::*)(__VA_ARGS__);	\
	friend Member GetPrivate(InClass##FuncName##Accessor);	\
};	\
template struct AccessPrivate<InClass##FuncName##Accessor, &InClass::FuncName>;

#define  CALL_PRIVATE_FUNC(InClass, InObj, FuncName, ...) ((InObj->*(GetPrivate(InClass##FuncName##Accessor())))(__VA_ARGS__))
/**----------------------End----------------------*/


/**----------------调用重载的保护/私有函数----------------*/
#define IMPLEMENT_GET_PRIVATE_FUNC_OVERLOAD(InClass, FuncName, FuncRet, ExpandName, ...)	\
struct InClass##FuncName##Accessor##ExpandName	\
{\
	using Member = FuncRet(InClass::*)(__VA_ARGS__);	\
	friend Member GetPrivate(InClass##FuncName##Accessor##ExpandName);	\
};	\
template struct AccessPrivate<InClass##FuncName##Accessor##ExpandName, &InClass::FuncName>;

#define  CALL_PRIVATE_FUNC_OVERLOAD(InClass, InObj, FuncName, ExpandName, ...) ((InObj->*(GetPrivate(InClass##FuncName##Accessor##ExpandName())))(__VA_ARGS__))
/**----------------------End----------------------*/

const TCHAR* ImportObjectProperties2(
	uint8* DestData,
	const TCHAR* SourceText,
	UStruct* ObjectStruct,
	UObject* SubobjectRoot,
	UObject* SubobjectOuter,
	FFeedbackContext* Warn,
	int32					Depth,
	int32					LineNumber,
	FObjectInstancingGraph* InInstanceGraph,
	const TMap<AActor*, AActor*>* ActorRemapper
);
bool GetBEGIN2(const TCHAR** Stream, const TCHAR* Match);
bool GetEND2(const TCHAR** Stream, const TCHAR* Match);
bool GetREMOVE2(const TCHAR** Stream, const TCHAR* Match);

/* 
* 通用小功能库
*/
UCLASS()
class COMMONUTIL_API UCommonUtilBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/*
	* 生成独一无二的ID(随机的、无序的)
	* @return			返回ID
	*/
	static uint32 GenerateUniqueID();

	/*
	* 生成ID(1,2,3...)
	* @return			返回ID
	*/
	static uint32 GenerateID();

	/**
	* ForExample:
	* 
	* UFUNCTION()
	* bool UFunctionTest(int32& aa, int32 bb);
	*
	* TTuple<int32, bool> Ret2;
	* UCommonUtilBPLibrary::CallFunction(this, TEXT("UFunctionTest"), Ret2, 200, 2);
	* UE_LOG(LogTemp, Warning, TEXT("%d, %d"), get<0>(Ret2), get<1>(Ret2));
	*/
	template<typename... Args, typename... Ret>
	static void CallFunction(UObject* CallbackTarget, FName FunctionName, TTuple<Ret...>& OutParams, Args...args);

	template<typename... Args, typename... Ret>
	static void CallFunction(UObject* CallbackTarget, FName FunctionName, TTuple<Ret...>& OutParams, TTuple<Args...> args);

	/*
	* 根据条件查找世界
	* @param InTriggerFunc		条件表达式
	* @param args				是否递归每个子文件夹
	* @return					返回查找到的世界
	*/
	static UWorld* ForEachWorld(TFunction<bool(UWorld*)> InTriggerFunc);

	/*
	* 根据类型查找资产
	* @param InClass			类型数组
	* @param bRecursivePaths	是否递归每个子文件夹
	* @return					返回查找到的资产数组
	*/
	UFUNCTION(BlueprintCallable, Category = "CommonUtil")
	static TArray<UObject*> FindObjectsByClass(TArray<UClass*> InClass, bool bRecursivePaths = false);

	/*
	* 根据类型及路径查找资产
	* @param InClass			类型数组
	* @param PackagePaths		文件夹路径数组
	* @param bRecursivePaths	是否递归每个子文件夹
	* @return					返回查找到的资产数组
	*/
	UFUNCTION(BlueprintCallable, Category = "CommonUtil")
	static TArray<UObject*> FindObjectsByClassAndPaths(TArray<UClass*> InClass, TArray<FString> PackagePaths, bool bRecursivePaths = false);

	/*
	* 返回当前鼠标位置的控件,可能是nullptr
	*/
	static TSharedPtr<SWidget> GetSlateWidgetUnderCursor();

	/*
	* 返回某个位置的控件,可能是nullptr
	*/
	static TSharedPtr<SWidget> GetSlateWidgetOnPosition(FVector2D InPosition);

	/*
	* 查找路径下所有相同后缀文件
	* @param InPath				查找路径
	* @param Extension			后缀
	* @param bRecursive			是否递归每个子文件夹
	* @param bDirectories		默认false
	* @return					返回查找到的文件路径数组
	*/
	UFUNCTION(BlueprintCallable, Category = "CommonUtil")
	static TArray<FString> FindFiles(const FString& InPath, const FString& Extension, bool bRecursive = false, bool bDirectories = false);

	/*
	* 将控件添加到视口上
	* @param InWidget			待添加的控件
	* @param InSize				控件大小
	*/
	//UFUNCTION(BlueprintCallable, Category = "CommonUtil")
	static void AddWidgetToViewport(TSharedPtr<SWidget> InWidget, FVector2D InPosition, FVector2D InSize, FVector2D InAlignment, int32 InZOrder = -1);

	UFUNCTION(BlueprintCallable, Category = "CommonUtil")
	static void AddWidgetToViewport2(FVector2D InPosition, FVector2D InSize, FVector2D InAlignment, int32 InZOrder = -1);

	#if WITH_EDITOR
	/*
	* 获得当前选中的UObject
	*/
	UFUNCTION(BlueprintCallable, Category = "CommonUtil")
	static TArray<UObject*> GetCurrentSelectedObjects();

	/*
	* 获得当前选中的AActor
	*/
	UFUNCTION(BlueprintCallable, Category = "CommonUtil")
	static TArray<AActor*> GetCurrentSelectedActors();
	#endif

	/*
	* 复制物体全部信息
	* @param Actors				待复制的物体数组
	* @param DestinationData	复制出来的文本信息,为空指针时将复制的文本给到粘贴板
	*/
	static void CopyActors(TArray<AActor*> Actors, FString* DestinationData = nullptr);
	
	/*
	* 根据文本信息粘贴物体
	* @param Actors				粘贴后的物体数组
	* @param DestinationData	粘贴的文本信息,为空指针时从粘贴板取文本
	*/
	static void PasteActors(TArray<AActor*>& OutPastedActors, FString* SourceData = nullptr);

	/**-------功能测试----------*/
	UFUNCTION(BlueprintCallable, Category = "CommonUtil")
	static void CopyActorsTest(TArray<AActor*> Actors);
	UFUNCTION(BlueprintCallable, Category = "CommonUtil")
	static void PasteActorsTest(TArray<AActor*> OutPastedActors);
	/**-------功能测试----------*/
};

template<std::size_t... Index, typename...Args, typename...Ret>
TTuple<Args..., Ret...> MakeTuple(Tmp::Indices<Index...> Ind, TTuple<Args...> args, TTuple<Ret...> OutParams)
{
	return TTuple<Args..., Ret...>(get<Index>(args)..., Ret()...);
}

template<typename...Args, typename...Ret>
void UCommonUtilBPLibrary::CallFunction(UObject* CallbackTarget, FName FunctionName, TTuple<Ret...>& OutParams, TTuple<Args...> args)
{
	if (UFunction* TriggerFunc = CallbackTarget->FindFunction(FunctionName))
	{
		uint8* OutPramsBuffer = (uint8*)&OutParams;

		TTuple<Args..., Ret...> Params = MakeTuple(Tmp::build_inds<sizeof...(Args)>::type(), args, OutParams);
		CallbackTarget->ProcessEvent(TriggerFunc, &Params);
		for (TFieldIterator<FProperty>it(TriggerFunc); it; ++it)
		{
			if (it->PropertyFlags & CPF_OutParm)
			{
				void* PropertyBuffer = it->ContainerPtrToValuePtr<void*>(&Params);
				it->CopyCompleteValue(OutPramsBuffer, PropertyBuffer);
				OutPramsBuffer += it->GetSize();
			}
		}
	}
}

template<typename...Args, typename...Ret>
void UCommonUtilBPLibrary::CallFunction(UObject* CallbackTarget, FName FunctionName, TTuple<Ret...>& OutParams, Args...args)
{
	if (UFunction* TriggerFunc = CallbackTarget->FindFunction(FunctionName))
	{
		uint8* OutPramsBuffer = (uint8*)&OutParams;

		TTuple<Args..., Ret...> Params(Forward<Args>(args)..., Ret()...);
		CallbackTarget->ProcessEvent(TriggerFunc, &Params);
		for (TFieldIterator<FProperty>it(TriggerFunc); it; ++it)
		{
			if (it->PropertyFlags & CPF_OutParm)
			{
				void* PropertyBuffer = it->ContainerPtrToValuePtr<void*>(&Params);
				it->CopyCompleteValue(OutPramsBuffer, PropertyBuffer);
				OutPramsBuffer += it->GetSize();
			}
		}
	}
}

/**
* 需要AddToRoot的代理类，构造自动AddToRoot,析构自动RemoveFromRoot
*/
template<typename T = UObject>
class COMMONUTIL_API FRootedUObject
{
public:
	FRootedUObject(T* InObj) : Obj(InObj)
	{
		Obj->AddToRoot();
	}
	~FRootedUObject()
	{
		if (Obj && Obj->IsRooted())
		{
			Obj->RemoveFromRoot();
		}
	}

	T* operator->()
	{
		return Obj;
	}
	T* Get() const
	{
		return Obj;
	}
private:
	T* Obj = nullptr;
};

