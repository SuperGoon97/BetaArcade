#pragma once

#include "Modules/ModuleManager.h"

class FRobotEditorRuntime : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
