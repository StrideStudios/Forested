using UnrealBuildTool;
using System.Collections.Generic;

public class ForestedTarget : TargetRules {
	public ForestedTarget(TargetInfo Target) : base(Target) {
		Type = TargetType.Game;
		//WindowsPlatform.PCHMemoryAllocationFactor = 400;
		CppStandard = CppStandardVersion.Default;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		
		ExtraModuleNames.Add("Forested");
	}
}
