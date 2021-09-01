// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class DateTimePickerPluginEditorTarget : TargetRules
{
	public DateTimePickerPluginEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
#if UE_4_24_OR_LATER
		DefaultBuildSettings = BuildSettingsVersion.V2;
#endif
		ExtraModuleNames.AddRange( new string[] { "DateTimePickerPlugin" } );
	}
}
