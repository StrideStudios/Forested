#include "ForestedLibrary.h"
#include "Engine/AssetManager.h"
#include "GameFramework/GameUserSettings.h" 
#include "Kismet/KismetMathLibrary.h" 
#include "Kismet/KismetStringLibrary.h" 

FAlphaBlend UForestedLibrary::MakeAlphaBlend(UCurveFloat* CustomCurve, const EAlphaBlendOption BlendOption, const float BlendTime) {
	FAlphaBlend Blend(BlendTime);
	Blend.SetCustomCurve(CustomCurve);
	Blend.SetBlendOption(BlendOption);
	return Blend;
}

float UForestedLibrary::Bounce(const float Time, const bool Reverse, const bool bBounce, const float TimeModify, const float Amplitude, const float Decay, const float Frequency, const TEnumAsByte<EEasingFunc::Type> EasingFunc, const TEnumAsByte<EEasingFunc::Type> ReverseEasingFunc, const float BlendExp, const int Steps) {
	float NewTime = Reverse ? FMath::Min(Time + TimeModify,1.f) : FMath::Max(Time + TimeModify,0.f);
	NewTime = Reverse ? 1 - NewTime : NewTime;
	const TEnumAsByte<EEasingFunc::Type> EaseFunction = Reverse ? ReverseEasingFunc : EasingFunc;
	float ReturnValue;
	if (NewTime >= 0.f && NewTime <= 1.f) {
		ReturnValue = UKismetMathLibrary::Ease(0.f, 1.f, NewTime, EaseFunction, BlendExp, Steps);
	} else {
		const float Amp = GetExponentFromEase(EaseFunction, BlendExp) * Amplitude * UKismetMathLibrary::Exp(-0.5 * Decay);
		const float W = Frequency * PI * 2;
		const float N = UKismetMathLibrary::Sin((NewTime - 1) * W);
		ReturnValue = 1 + (bBounce ? -Amp : Amp) * ((bBounce ? FMath::Abs(N) : N) / UKismetMathLibrary::Exp(Decay * (NewTime - 1.5)) / W);
	}
	return Reverse ? 1 - ReturnValue : ReturnValue;
}

float UForestedLibrary::GetExponentFromEase(const TEnumAsByte<EEasingFunc::Type> EasingFunc, const float BlendExp) {
	switch (EasingFunc) {
	case EEasingFunc::Type::EaseIn:
		return BlendExp;
	case EEasingFunc::Type::EaseOut:
		return FMath::Pow(0.25f, BlendExp);
	case EEasingFunc::Type::ExpoIn:
		return EULERS_NUMBER;
	case EEasingFunc::Type::ExpoOut:
		return FMath::Pow(0.25f, EULERS_NUMBER);
	case EEasingFunc::Type::SinusoidalIn:
		return 2.f;
	case EEasingFunc::Type::Linear: case EEasingFunc::Type::Step:
		return 1.f;
	default:
		return 0.f;
	}
}

void UForestedLibrary::SetQualitySetting(const EScalabilitySettings Setting, const int32 Quality){
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	if (!Settings) return;
	switch (Setting) {
	case EScalabilitySettings::ResolutionQuality:
		Settings->SetResolutionScaleValueEx(static_cast<float>(Quality));
		break;
	case EScalabilitySettings::ViewDistanceQuality:
		Settings->SetViewDistanceQuality(Quality);
		break;
	case EScalabilitySettings::AntiAliasingQuality:
		Settings->SetAntiAliasingQuality(Quality);
		break;
	case EScalabilitySettings::ShadowQuality:
		Settings->SetShadowQuality(Quality);
		break;
	case EScalabilitySettings::PostProcessQuality:
		Settings->SetPostProcessingQuality(Quality);
		break;
	case EScalabilitySettings::TextureQuality:
		Settings->SetTextureQuality(Quality);
		break;
	case EScalabilitySettings::EffectQuality:
		Settings->SetVisualEffectQuality(Quality);
		break;
	case EScalabilitySettings::FoliageQuality:
		Settings->SetFoliageQuality(Quality);
		break;
	case EScalabilitySettings::ShadingQuality:
		Settings->SetShadingQuality(Quality);
		break;
	default: break;
	}
	Settings->ConfirmVideoMode();
	Settings->ApplySettings(true);
}

void UForestedLibrary::SetVariable(const FString& Variable) {
	FString Key;
	FString Value;
	UKismetStringLibrary::Split(Variable, "=", Key, Value);
	if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*Key)) {
		CVar->Set(*Value, ECVF_SetByCode); //ECVF_SetByGameSetting
	}
}

void UForestedLibrary::SetVariables(const TArray<FString>& Variables) {
	for (FString Variable : Variables) {
		FString Key;
		FString Value;
		UKismetStringLibrary::Split(Variable, "=", Key, Value);
		if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*Key)) {
			CVar->Set(*Value, ECVF_SetByCode); //ECVF_SetByGameSetting
		}
	}
}