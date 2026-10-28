// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PartEditor : ModuleRules
{
	public PartEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
                "RobotEditor/PartEditor/Public",
			}
			);


		PrivateIncludePaths.AddRange(
			new string[] {
                "RobotEditor/PartEditor/Private",
			}
			);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "Niagara",
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
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "MainFrame",
                "LevelEditor",
                "ContentBrowser",
                "PropertyEditor",
                "InteractiveToolsFramework",
                "GeometryCollectionEngine",
                "RobotEditorRuntime",
                "EditorUtils",
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
