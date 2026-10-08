// Copyright Epic Games, Inc. All Rights Reserved.

#include "PartEditorCommands.h"

#define LOCTEXT_NAMESPACE "FPartEditorModule"

void FPartEditorCommands::RegisterCommands() {
	UI_COMMAND(OpenPluginWindow, "PartEditor", "Bring up Part Editor window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
