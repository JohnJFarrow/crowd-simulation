// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

// Setup based on https://forums.unrealengine.com/t/unresolved-reference-fmassfragment/572465

public class MyProject2 : ModuleRules
{
	public MyProject2(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(new string[] { "../Ext/nanoflann/include" });

		PublicDependencyModuleNames.AddRange(new string[] { 
            "Core", 
            "CoreUObject", 
            "Engine", 
            "InputCore", 
            "EnhancedInput", 
            "NavigationSystem", 
            "AIModule", 
            "StructUtils"
        } );

		PrivateDependencyModuleNames.AddRange(new string[] { 
            "MassAIBehavior",
            "MassEntity",
            "MassCommon",
            "MassCore",
            "MassCrowd",
            "MassActors",
            "MassAIDebug",
            "MassGameplayDebug",
            "MassLOD",
            "MassMovement",
            "MassNavigation",
            "MassRepresentation",
            "MassReplication",
            "MassSpawner",
            "MassSimulation",
            "MassSignals", });
    }
}