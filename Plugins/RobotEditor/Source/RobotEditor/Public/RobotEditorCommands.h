// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"
#include "RobotEditorStyle.h"

class FRobotEditorCommands : public TCommands<FRobotEditorCommands> {
public:

	FRobotEditorCommands()
		: TCommands<FRobotEditorCommands>(TEXT("RobotEditor"), NSLOCTEXT("Contexts", "RobotEditor", "RobotEditor Plugin"), NAME_None, FRobotEditorStyle::GetStyleSetName())
	{}

	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};
