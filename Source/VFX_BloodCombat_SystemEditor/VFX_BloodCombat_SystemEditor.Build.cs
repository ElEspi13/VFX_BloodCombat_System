// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VFX_BloodCombat_SystemEditor : ModuleRules
{
	public VFX_BloodCombat_SystemEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UnrealEd",
				"VFX_BloodCombat_System",
			}
		);
	}
}
