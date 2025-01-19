using UnrealBuildTool;

public class Forested : ModuleRules  {
	public Forested(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", "CoreUObject", "Engine", 
			"InputCore", "EnhancedInput", 
			"UMG", "Slate", "SlateCore", 
			"Niagara", "PhysicsCore",
			"KismetCompiler", "BlueprintGraph", "UnrealEd"
		});
	}
}
