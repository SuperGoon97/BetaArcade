// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

static const FName RobotEditorTabName("RobotEditor");
static const FName PartEditorTabName("PartEditor");
static const FName EditorUnifiedWindowLayoutVer("EditorUnifiedLayout_v1.0");

class FRobotEditorModule : public IModuleInterface {
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

    void OnEngineInitialized();
	void PluginButtonClicked();

private:

	void RegisterMenus();

private:
	TSharedPtr<class FUICommandList> PluginCommands;
};
