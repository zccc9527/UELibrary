// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "CommonUtilBPLibrary.generated.h"

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
* ͨ��С���ܿ�
*/
UCLASS()
class COMMONUTIL_API UCommonUtilBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/*
	* �������Ͳ����ʲ�
	* @param InClass			��������
	* @param bRecursivePaths	�Ƿ�ݹ�ÿ�����ļ���
	* @return					���ز��ҵ����ʲ�����
	*/
	UFUNCTION(BlueprintCallable, Category = "CommonUtil")
	static TArray<UObject*> FindObjectsByClass(TArray<UClass*> InClass, bool bRecursivePaths = false);

	/*
	* �������ͼ�·�������ʲ�
	* @param InClass			��������
	* @param PackagePaths		�ļ���·������
	* @param bRecursivePaths	�Ƿ�ݹ�ÿ�����ļ���
	* @return					���ز��ҵ����ʲ�����
	*/
	UFUNCTION(BlueprintCallable, Category = "CommonUtil")
	static TArray<UObject*> FindObjectsByClassAndPaths(TArray<UClass*> InClass, TArray<FString> PackagePaths, bool bRecursivePaths = false);

	/*
	* ���ص�ǰ���λ�õĿؼ�,������nullptr
	*/
	static TSharedPtr<SWidget> GetSlateWidgetUnderCursor();

	/*
	* ����·����������ͬ��׺�ļ�
	* @param InPath				����·��
	* @param Extension			��׺
	* @param bRecursive			�Ƿ�ݹ�ÿ�����ļ���
	* @param bDirectories		Ĭ��false
	* @return					���ز��ҵ����ļ�·������
	*/
	UFUNCTION(BlueprintCallable, Category = "CommonUtil")
	static TArray<FString> FindFiles(const FString& InPath, const FString& Extension, bool bRecursive = false, bool bDirectories = false);

	/*
	* ���ؼ���ӵ��ӿ���
	* @param InWidget			����ӵĿؼ�
	* @param InSize				�ؼ���С
	*/
	//UFUNCTION(BlueprintCallable, Category = "CommonUtil")
	static void AddWidgetToViewport(TSharedPtr<SWidget> InWidget, FVector2D InPosition, FVector2D InSize, FVector2D InAlignment, int32 InZOrder = -1);

	UFUNCTION(BlueprintCallable, Category = "CommonUtil")
	static void AddWidgetToViewport2(FVector2D InPosition, FVector2D InSize, FVector2D InAlignment, int32 InZOrder = -1);

	#if WITH_EDITOR
	/*
	* ��õ�ǰѡ�е�UObject
	*/
	UFUNCTION(BlueprintCallable, Category = "CommonUtil")
	static TArray<UObject*> GetCurrentSelectedObjects();

	/*
	* ��õ�ǰѡ�е�AActor
	*/
	UFUNCTION(BlueprintCallable, Category = "CommonUtil")
	static TArray<AActor*> GetCurrentSelectedActors();
	#endif

	UFUNCTION(BlueprintCallable, Category = "CommonUtil")
	static bool SaveToJson(UObject* SaveObject, const FString& Path);
	UFUNCTION(BlueprintCallable, Category = "CommonUtil")
	static bool LoadFromJson(UObject* OutObject, const FString& Path);

	UFUNCTION(BlueprintCallable, Category = "CommonUtil")
	static bool SaveObject(UObject* SaveObject, const FString& Path);
	UFUNCTION(BlueprintCallable, Category = "CommonUtil")
	static bool LoadObject(UObject* OutObject, const FString& Path);

	//UFUNCTION(BlueprintCallable, Category = "CommonUtil")
	static void CopyActors(TArray<AActor*>& Actors, FString* DestinationData = nullptr);
	//UFUNCTION(BlueprintCallable, Category = "CommonUtil")
	static void PasteActors(TArray<AActor*>& OutPastedActors, UWorld* InWorld, FString* SourceData = nullptr);

	UFUNCTION(BlueprintCallable, Category = "CommonUtil")
	static void CopyActorsTest(TArray<AActor*> Actors);
	UFUNCTION(BlueprintCallable, Category = "CommonUtil")
	static void PasteActorsTest(TArray<AActor*> OutPastedActors, UObject* InWorld);
};

template<typename T = UObject>
class FRootedUObject
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
private:
	T* Obj = nullptr;
};

namespace Tmp
{
	template<typename T, T v>
	struct COMMONUTIL_API integral_constant
	{
		static constexpr T value = v;
		using value_type = T;
		using type = integral_constant; //ָ�������ͣ��𵽼��ʹ�õ�����
		constexpr value_type operator()() const noexcept { return value; } //�������������
		constexpr operator value_type() const noexcept { return value; } //����ת�������Խ���������ʽת��Ϊvalue_type����ʹ��
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

	//�����ڼ��if
	template<bool B, typename T, typename F>
	struct COMMONUTIL_API conditional;
	template<typename T, typename F>
	struct COMMONUTIL_API conditional<false, T, F> : public type_identity<F> {};
	template<typename T, typename F>
	struct COMMONUTIL_API conditional<true, T, F> : public type_identity<T> {};
	template<bool B, typename T, typename F>
	using conditional_t = typename conditional<B, T, F>::type;

	//�жϱ�����С
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

	//���ģ�������Ƿ��ǿ��Կ�����(ֻ�жϸ�ֵ������ܷ�ʹ��)
	template <typename T, typename = void>
	struct COMMONUTIL_API is_copyable : false_type {};
	template <typename T>
	struct COMMONUTIL_API is_copyable<T, void_t<decltype(declval<T&>() = declval<const T&>())>> : true_type {};
	template <typename T>
	constexpr bool is_copyable_v = is_copyable<T>::value;

	//���ģ�������Ƿ��ǿ��Կ�����(ֻ�жϸ�ֵ������ܷ�ʹ��)
	/*template <typename T, typename = void>
	struct is_copyable_assign : false_type {};
	template <typename T>
	struct is_copyable_assign<T, void_t<decltype(std::declval<T&>(std::declval<const T&>()))>> : true_type {};
	template <typename T>
	constexpr bool is_copyable_assign_v = is_copyable_assign<T>::value;*/

	//�жϸ������Ƿ����Ĭ�Ϲ��캯��
	template<typename T, typename U = void_t<>>
	struct COMMONUTIL_API Has_DefaultConstructor : false_type {};
	template<typename T>
	struct COMMONUTIL_API Has_DefaultConstructor<T, void_t<decltype(T())>> : true_type {};
	template<typename T>
	constexpr bool Has_DefaultConstructor_v = Has_DefaultConstructor<T>::value;

	//�ж�����From�ܷ���ʽת��Ϊ����To
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

	//�ж�һ�������Ƿ���������
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

	//�ж�Base�Ƿ���Derived�ĸ���
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

#ifndef DEFINE_HAS_TYPE_MEM
#define DEFINE_HAS_TYPE_MEM(type)	\
template<typename T, typename U = Tmp::void_t<>>	\
struct _HAVE_TYPE_MEMBER_##type : Tmp::false_type {};	\
template<typename T>	\
struct _HAVE_TYPE_MEMBER_##type<T, Tmp::void_t<typename T::type>> : Tmp::true_type {};
#endif

#ifndef DEFINE_HAS_VALUE_MEM
#define DEFINE_HAS_VALUE_MEM(value)	\
template<typename T, typename U = Tmp::void_t<>>	\
struct _HAVE_VALUE_MEMBER_##value : Tmp::false_type {};	\
template<typename T>	\
struct _HAVE_VALUE_MEMBER_##value<T, Tmp::void_t<decltype(T::value)>> : Tmp::true_type {};
#endif

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