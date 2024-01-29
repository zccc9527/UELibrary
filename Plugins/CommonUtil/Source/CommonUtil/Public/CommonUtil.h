// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FOnUObjectCreateListener;

class FCommonUtilModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

//protected:
//	FOnUObjectCreateListener* OnUObjectCreateListener = nullptr;
};
