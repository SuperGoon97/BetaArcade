// Copyright Epic Games, Inc. All Rights Reserved.

#include "RobotEditorCommands.h"

#define LOCTEXT_NAMESPACE "FRobotEditorModule"

void FRobotEditorCommands::RegisterCommands() {
	UI_COMMAND(OpenPluginWindow, "RobotEditor", "Bring up RobotEditor window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
