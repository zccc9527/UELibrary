#include "CommonFunctionalClass.h"

FOnUObjectCreateListener::FOnUObjectCreateListener()
{
	GUObjectArray.AddUObjectCreateListener(this);
}

FOnUObjectCreateListener::~FOnUObjectCreateListener()
{
	GUObjectArray.RemoveUObjectCreateListener(this);
}

FOnUObjectDeleteListener::FOnUObjectDeleteListener()
{
	GUObjectArray.AddUObjectDeleteListener(this);
}

FOnUObjectDeleteListener::~FOnUObjectDeleteListener()
{
	GUObjectArray.RemoveUObjectDeleteListener(this);
}
