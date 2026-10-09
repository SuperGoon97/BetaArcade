// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RobotEditorRuntime : ModuleRules
{
	public RobotEditorRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
                "RobotEditor/RobotEditorRuntime/Public"
			}
			);


		PrivateIncludePaths.AddRange(
			new string[] {
                "RobotEditor/RobotEditorRuntime/Private"
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
			}
			);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
