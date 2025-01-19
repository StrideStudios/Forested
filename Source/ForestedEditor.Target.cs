using UnrealBuildTool;
using System.Collections.Generic;

public class ForestedEditorTarget : TargetRules {
	public ForestedEditorTarget(TargetInfo Target) : base(Target) {
		Type = TargetType.Editor;
		//WindowsPlatform.PCHMemoryAllocationFactor = 400;
		CppStandard = CppStandardVersion.Default;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		
		ExtraModuleNames.Add("Forested");
	}
}
