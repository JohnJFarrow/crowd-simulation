// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

[SupportedPlatforms(UnrealPlatformClass.Server)]
public class MZServerTarget : TargetRules
{
	public MZServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;

		ExtraModuleNames.AddRange(new string[] { "MZ" });
		MZGameTarget.ApplySharedTargetSettings(this);

		bUseChecksInShipping = true;
	}
}
