// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RobotEditor : ModuleRules
{
	public RobotEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
                "RobotEditor/RobotEditor/Public",
			}
			);


		PrivateIncludePaths.AddRange(
			new string[] {
                "RobotEditor/RobotEditor/Private",
			}
			);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"InputCore",
				"EditorFramework",
				"UnrealEd",
				"ToolMenus",
                "Niagara",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "MainFrame",
                "LevelEditor",
                "ContentBrowser",
                "PropertyEditor",
                "InteractiveToolsFramework",
                "RobotEditorRuntime"
				// ... add private dependencies that you statically link with here ...
			}
			);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
