#include "Lights/TimedLight.h"
#include "Sky.h"
#include "Components/PointLightComponent.h" 
#include "Components/RectLightComponent.h" 
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SpotLightComponent.h"
#include "Kismet/GameplayStatics.h"

ATimedLight::ATimedLight() {
	PrimaryActorTick.bCanEverTick = false;
	
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));
	RootComponent = Root;
}

void ATimedLight::BeginPlay() {
	Super::BeginPlay();
	SKY->OnSkyUpdate.AddUniqueDynamic(this, &ATimedLight::RefreshLight);
	SKY->OnSkyUpdate.AddUniqueDynamic(this, &ATimedLight::ReceiveRefreshLight);
	
	RefreshLight(0.f, SKY);
}

void ATimedLight::RefreshLight(const float DeltaTime, const ASky* Sky) {
	if (!IsEnabled()) {
		for (ULightComponent* LightComponent : GetLightComponents())
			LightComponent->SetVisibility(false);
		return;
	}
	if (!Sky) return;
	const float SunHeight = Sky->GetSunHeight();
	const bool bShouldBeEnabled = SunHeight >= GetStartTime() && SunHeight <= GetEndTime();
	const float Intensity = IntensityCurve ? IntensityCurve->GetFloatValue(SunHeight) : 1.f;
	for (ULightComponent* LightComponent : GetLightComponents()) {
		LightComponent->SetVisibility(bShouldBeEnabled && Intensity > 0.f);
		if (!LightComponent->IsVisible()) continue;
		LightComponent->SetIntensity(LightComponent->Intensity * Intensity);
	}
}

void ATimedLight::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ATimedLight, StartTime) ||
		PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ATimedLight, EndTime)) {
		RefreshLight(0.f, SKY);
	}

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ATimedLight, bEnabled)) {
		// Ensure Lights are Disabled when unchecked
		SetEnabled(IsEnabled());
	}
	
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void ATimedLight::Enable()  {
	bEnabled = true;
	RefreshLight(0.f, SKY);
}

void ATimedLight::Disable()  {
	bEnabled = false;
	// Custom Update to ensure lights are disabled
	for (ULightComponent* LightComponent : GetLightComponents()) {
		LightComponent->SetVisibility(false);
	}
}

ATimedPointLight::ATimedPointLight() {
	PrimaryActorTick.bCanEverTick = false;

	PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("Point Light"));
	PointLight->SetupAttachment(GetRootComponent());
	PointLight->SetLightingChannels(true, true, false);
}

TArray<ULightComponent*> ATimedPointLight::GetLightComponents() const { return { PointLight }; }

ATimedRectLight::ATimedRectLight() {
	PrimaryActorTick.bCanEverTick = false;

	RectLight = CreateDefaultSubobject<URectLightComponent>(TEXT("Rect Light"));
	RectLight->SetupAttachment(GetRootComponent());
	RectLight->SetLightingChannels(true, true, false);
}

TArray<ULightComponent*> ATimedRectLight::GetLightComponents() const { return { RectLight }; }

ATimedSpotLight::ATimedSpotLight() {
	PrimaryActorTick.bCanEverTick = false;

	SpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Spot Light"));
	SpotLight->SetupAttachment(GetRootComponent());
	SpotLight->SetLightingChannels(true, true, false);
}

TArray<ULightComponent*> ATimedSpotLight::GetLightComponents() const { return { SpotLight }; }
