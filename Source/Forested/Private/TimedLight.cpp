#include "TimedLight.h"

#include "LevelDefaults.h"
#include "Sky.h"
#include "Components/ArrowComponent.h"
#include "Components/PointLightComponent.h" 
#include "Components/RectLightComponent.h" 
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SpotLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

ATimedLight::ATimedLight() {
	PrimaryActorTick.bCanEverTick = false;
	
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));
	RootComponent = Root;
}

void ATimedLight::BeginPlay() {
	Super::BeginPlay();
	SKY->OnSkyUpdate.AddUniqueDynamic(this, &ATimedLight::RefreshLight);
	RefreshLight(0.f, SKY);
}

void ATimedLight::OnConstruction(const FTransform& Transform) {
	const ASky* Sky = Cast<ASky>(UGameplayStatics::GetActorOfClass(this, ASky::StaticClass()));
	if (!Sky) return;
	RefreshLight(0.f, Sky);
}

void ATimedLight::RefreshLight_Implementation(const float DeltaTime, const ASky* Sky) {
	if (!Sky) return;
	const float SunHeight = Sky->GetSunHeight();
	const bool bShouldBeEnabled = SunHeight >= MinTime && SunHeight <= MaxTime;
	const float Intensity = IntensityCurve ? IntensityCurve->GetFloatValue(SunHeight) : 1.f;
	for (ULightComponent* LightComponent : GetLightComponents()) {
		LightComponent->SetVisibility(bShouldBeEnabled && Intensity > 0.f);
		if (!LightComponent->IsVisible()) continue;
		LightComponent->SetIntensity(LightComponent->Intensity * Intensity);
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
