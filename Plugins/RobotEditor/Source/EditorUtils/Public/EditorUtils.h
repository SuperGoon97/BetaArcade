#pragma once

#include "Modules/ModuleManager.h"

class FEditorUtils : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
