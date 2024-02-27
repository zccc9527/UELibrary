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