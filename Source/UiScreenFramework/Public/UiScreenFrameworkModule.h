// Copyright People Can Fly. All Rights Reserved."

#pragma once

#include "Modules/ModuleManager.h"

class FUiScreenFrameworkModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
