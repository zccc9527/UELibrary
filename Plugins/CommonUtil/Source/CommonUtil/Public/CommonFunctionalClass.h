#pragma once

#include "CoreMinimal.h"

/**
* UObject物体创建时的监听者
*/
class COMMONUTIL_API FOnUObjectCreateListener : public FUObjectArray::FUObjectCreateListener
{
public:
	FOnUObjectCreateListener();
	~FOnUObjectCreateListener();
	/**
	* UObject物体创建时调用的函数
	*/
	virtual void NotifyUObjectCreated(const class UObjectBase* Object, int32 Index) = 0;
	/*{
		if (Object)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s, U:%d I:%d"), *Object->GetFName().ToString(), Object->GetUniqueID(), Index);
			UE_LOG(LogTemp, Warning, TEXT("%s"), *GUObjectArray.IndexToObject(Index)->Object->GetFName().ToString());
		}
	}*/
	virtual void OnUObjectArrayShutdown() {}
};

/**
* UObject物体销毁时的监听者
*/
class COMMONUTIL_API FOnUObjectDeleteListener : public FUObjectArray::FUObjectDeleteListener
{
public:
	FOnUObjectDeleteListener();
	~FOnUObjectDeleteListener();
	/**
	* UObject物体销毁时调用的函数
	*/
	virtual void NotifyUObjectDeleted(const class UObjectBase* Object, int32 Index) = 0;
	virtual void OnUObjectArrayShutdown() {}
};