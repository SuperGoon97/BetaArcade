// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"
#include "PartEditorStyle.h"

class FPartEditorCommands : public TCommands<FPartEditorCommands> {
public:

	FPartEditorCommands()
		: TCommands<FPartEditorCommands>(TEXT("Part Editor"), NSLOCTEXT("Contexts", "PartEditor", "PartEditor Plugin"), NAME_None, FPartEditorStyle::GetStyleSetName())
	{}

	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};
