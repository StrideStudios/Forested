#pragma once

#include "Forested/ForestedMinimal.h"
#include "AlphaBlend.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "ForestedLibrary.generated.h"

class UItem;
class UBoxComponent;

UENUM(BlueprintType)
enum class EConditionType : uint8 {
	//active when the condition is true
	ActiveWhenTrue UMETA(DisplayName = "Active When True"),
	//active when the condition is false
	ActiveWhenFalse UMETA(DisplayName = "Active When False"),
	//ignore THIS condition
	Ignore UMETA(DisplayName = "Ignore")
};

UENUM(BlueprintType)
enum class EScalabilitySettings : uint8 {
	None UMETA(DisplayName = "None"),
	ResolutionQuality UMETA(DisplayName = "Resolution Quality"),
	ViewDistanceQuality UMETA(DisplayName = "View Distance Quality"),
	AntiAliasingQuality UMETA(DisplayName = "Anti-Aliasing Quality"),
	ShadowQuality UMETA(DisplayName = "Shadow Quality"),
	PostProcessQuality UMETA(DisplayName = "Post-Process Quality"),
	TextureQuality UMETA(DisplayName = "Texture Quality"),
	EffectQuality UMETA(DisplayName = "Effects Quality"),
	FoliageQuality UMETA(DisplayName = "Foliage Quality"),
	ShadingQuality UMETA(DisplayName = "Shading Quality")
};

UCLASS()
class FORESTED_API UForestedLibrary : public UBlueprintFunctionLibrary {
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utilities|Struct")
	static FAlphaBlend MakeAlphaBlend(UCurveFloat* CustomCurve, EAlphaBlendOption BlendOption, float BlendTime);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(AdvancedDisplay = "2"), Category = "Math|Interpolation")
	static float Bounce(float Time, bool Reverse = false, bool bBounce = false, float TimeModify = 0.f, float Amplitude = 1.f, float Decay = 5.f, float Frequency = 1.f, TEnumAsByte<EEasingFunc::Type> EasingFunc = EEasingFunc::Linear, TEnumAsByte<EEasingFunc::Type> ReverseEasingFunc = EEasingFunc::Linear, float BlendExp = 2.f, int Steps = 2);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (AdvancedDisplay = "2"), Category = "Math|Interpolation")
	static float GetExponentFromEase(TEnumAsByte<EEasingFunc::Type> EasingFunc = EEasingFunc::Linear, float BlendExp = 2.f);

	UFUNCTION(BlueprintCallable, Category = "Development")
	static void SetQualitySetting(EScalabilitySettings Setting, int32 Quality);

	UFUNCTION(BlueprintCallable, Category = "Development")
	static void SetVariable(const FString& Variable);
	
	UFUNCTION(BlueprintCallable, Category = "Development")
	static void SetVariables(const TArray<FString>& Variables);

};
