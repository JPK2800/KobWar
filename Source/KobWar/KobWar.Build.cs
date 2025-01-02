// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class KobWar : ModuleRules
{
	public KobWar(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "NavigationSystem", "ClientAuthoritativeCharacterSystem", "UMG" });

        PrivateDependencyModuleNames.AddRange(new string[] { });

    }
}
