// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class EditorUtils : ModuleRules
{
	public EditorUtils(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
                "RobotEditor/EditorUtils/Public"
			}
			);


		PrivateIncludePaths.AddRange(
			new string[] {
                "RobotEditor/EditorUtils/Private"
			}
			);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
			);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
                "Niagara",
				"CoreUObject",
				"Engine",
                "Slate",
				"SlateCore",
                "UnrealEd",
                "MainFrame",
                "LevelEditor",
                "ContentBrowser",
                "InputCore",
                "PropertyEditor",
                "ToolMenus",
            }
			);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
