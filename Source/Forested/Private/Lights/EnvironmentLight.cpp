#include "Lights/EnvironmentLight.h"
#include "Sky.h"
#include "Components/ArrowComponent.h"
#include "Components/PointLightComponent.h" 
#include "Components/RectLightComponent.h" 
#include "Components/SkyAtmosphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

//TODO: add a condition or whatever when disabled it is removed from the sky update pool
AEnvironmentLight::AEnvironmentLight() {
	PrimaryActorTick.bCanEverTick = false;
	
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));
	RootComponent = Root;
}

void AEnvironmentLight::BeginPlay() {
	Super::BeginPlay();
	SKY->OnSkyUpdate.AddUniqueDynamic(this, &AEnvironmentLight::RefreshLight);
	SKY->OnSkyUpdate.AddUniqueDynamic(this, &AEnvironmentLight::ReceiveRefreshLight);

	RefreshLight(0.f, SKY);
}

void AEnvironmentLight::RefreshLight(const float DeltaTime, const ASky* Sky) {
	if (!IsEnabled()) {
		for (ULightComponent* LightComponent : GetLightComponents())
			LightComponent->SetVisibility(false);
		return;
	}
	if (!Sky) return;
	const float Intensity = (Sky->IsNight() ? NightIntensity : DayIntensity) / 100.f * (IntensityCurve ? IntensityCurve->GetFloatValue(Sky->GetSunHeight()) : 1.f);
	const float Temperature = Sky->IsNight() ? NightTemperature : DayTemperature;
	const FLinearColor SkyColor = Sky->IsNight() ? Sky->Moon->GetLightColor() : Sky->SkyAtmosphere->GetAtmosphereTransmitanceOnGroundAtPlanetTop(Sky->Sun);
	for (ULightComponent* LightComponent : GetLightComponents()) {
		LightComponent->SetVisibility(true);
		LightComponent->SetIntensity(Intensity);
		LightComponent->SetTemperature(Temperature);
		LightComponent->SetLightColor(SkyColor);
	}
}

void AEnvironmentLight::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(AEnvironmentLight, bEnabled) ||
		PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(AEnvironmentLight, DayIntensity) ||
		PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(AEnvironmentLight, NightIntensity) ||
		PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(AEnvironmentLight, DayTemperature) ||
		PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(AEnvironmentLight, NightTemperature)) {
		RefreshLight(0.f, SKY);
	}
	
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void AEnvironmentLight::Enable()  {
	bEnabled = true;
	RefreshLight(0.f, SKY);
}

void AEnvironmentLight::Disable()  {
	bEnabled = false;
	RefreshLight(0.f, SKY);
}

AEnvironmentPointLight::AEnvironmentPointLight() {
	PrimaryActorTick.bCanEverTick = false;

	PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("Point Light"));
	PointLight->SetupAttachment(GetRootComponent());
	PointLight->SetLightingChannels(false, true, false);
	PointLight->SetIntensityUnits(ELightUnits::Unitless);
	PointLight->bUseInverseSquaredFalloff = false;
	PointLight->SetLightFalloffExponent(2.f);
	PointLight->SetCastShadows(true);
	PointLight->SetSpecularScale(0.f);
}

TArray<ULightComponent*> AEnvironmentPointLight::GetLightComponents() const { return { PointLight }; }

AEnvironmentRectLight::AEnvironmentRectLight() {
	PrimaryActorTick.bCanEverTick = false;

	RectLight = CreateDefaultSubobject<URectLightComponent>(TEXT("Rect Light"));
	RectLight->SetupAttachment(GetRootComponent());
	RectLight->SetLightingChannels(false, true, false);
	RectLight->SetIntensityUnits(ELightUnits::Candelas);
}

TArray<ULightComponent*> AEnvironmentRectLight::GetLightComponents() const { return { RectLight }; }

AEnvironmentBounceLight::AEnvironmentBounceLight() {
	PrimaryActorTick.bCanEverTick = false;

	BounceLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("Bounce Light"));
	BounceLight->SetupAttachment(GetRootComponent());
	BounceLight->SetLightingChannels(false, true, false);
	BounceLight->SetIntensityUnits(ELightUnits::Unitless);
	BounceLight->bUseInverseSquaredFalloff = false;
	BounceLight->SetLightFalloffExponent(2.f);
	BounceLight->SetCastShadows(false);
	BounceLight->SetSpecularScale(0.f);
	ForwardDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("Forward Direction"));
	ForwardDirection->SetupAttachment(GetRootComponent());
	ForwardDirection->SetArrowColor(FLinearColor(1.f,1.f,1.f));
	ForwardDirection->ScreenSize = 0.005f;
	ForwardDirection->bIsScreenSizeScaled = true;
	LightDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("Light Direction"));
	LightDirection->SetupAttachment(GetRootComponent());
	LightDirection->SetArrowColor(FLinearColor(1.f,0.5f,0.f));
	LightDirection->ScreenSize = 0.005f;
	LightDirection->bIsScreenSizeScaled = true;
}

void AEnvironmentBounceLight::BeginPlay() {
	Super::BeginPlay();
	BounceLight->SetIntensity(NewIntensity);
	BounceLight->SetWorldLocation(NewLocation);
	BounceLight->SetVisibility(BounceLight->Intensity > 0.f);
}

void AEnvironmentBounceLight::RefreshLight(const float DeltaTime, const ASky* Sky) {
	if (!IsEnabled()) {
		Super::RefreshLight(DeltaTime, Sky);
		return;
	}
	if (!Sky) return;
#if WITH_EDITOR
	LightDirection->SetWorldRotation(Sky->Sun->GetComponentRotation());
#endif

	BounceLight->SetWorldLocation(UKismetMathLibrary::VInterpTo(BounceLight->GetComponentLocation(), NewLocation, DeltaTime, 5.f));
	BounceLight->SetIntensity(UKismetMathLibrary::FInterpTo(BounceLight->Intensity, NewIntensity, DeltaTime, 5.f));
	BounceLight->SetVisibility(BounceLight->Intensity > 0.f);
	
	//if in editor world just set the location
#if WITH_EDITOR
	if (IN_EDITOR_WORLD) {
		BounceLight->SetWorldLocation(NewLocation);
		BounceLight->SetIntensity(NewIntensity);
		BounceLight->SetVisibility(BounceLight->Intensity > 0.f);
	}
#endif
	
	const float BounceDirectionalIntensity = UKismetMathLibrary::Dot_VectorVector(Sky->GetLightDirection(), GetActorForwardVector());
	if (BounceDirectionalIntensity <= 0.f) {
		NewIntensity = 0.f;
		return;
	}
		
	FHitResult HitResult;
	FCollisionQueryParams Params(TEXT("LineTraceSingleForObjects"), SCENE_QUERY_STAT_ONLY(KismetTraceUtils), true);
	Params.bReturnPhysicalMaterial = false;
	Params.bReturnFaceIndex = false;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActors(IgnoredActors);

#if WITH_EDITOR
	if(DrawDebug) {
		UKismetSystemLibrary::DrawDebugLine(this, GetActorLocation() - Sky->GetLightDirection() * BounceStartDistance, GetActorLocation(), FLinearColor(1.f,0.f,0.f));
		UKismetSystemLibrary::DrawDebugLine(this, GetActorLocation(), GetActorLocation() + Sky->GetLightDirection() * BounceEndDistance, FLinearColor(0.f,1.f,0.f));
	}
#endif
	
	//if found a hit before the light, don't show
	if (GetWorld()->LineTraceSingleByObjectType(HitResult, GetActorLocation() - Sky->GetLightDirection() * BounceStartDistance, GetActorLocation(), ECC_WorldStatic, Params)) {
		NewIntensity = 0.f;
		return;
	}
	
	//if haven't hit anything after the light, don't show (also cuts off if hit is too close)
	if (!GetWorld()->LineTraceSingleByObjectType(HitResult, GetActorLocation(), GetActorLocation() + Sky->GetLightDirection() * BounceEndDistance, ECC_WorldStatic, Params) || HitResult.Time <= 0.05f) {
		NewIntensity = 0.f;
		return;
	}
	
	NewLocation = HitResult.Location + Sky->GetLightDirection() * -1.f * BounceSeparate + HitResult.Normal * NormalBounceSeparate;
	NewIntensity = (Sky->IsNight() ? GetNightIntensity() : GetDayIntensity()) / 100.f * (GetIntensityCurve() ? GetIntensityCurve()->GetFloatValue(Sky->GetSunHeight()) : 1.f) * FMath::Clamp(BounceDirectionalIntensity, 0.f, 1.f);
	BounceLight->SetTemperature(Sky->IsNight() ? GetNightTemperature() : GetDayTemperature());
	BounceLight->SetLightColor(Sky->IsNight() ? Sky->Moon->GetLightColor() : Sky->SkyAtmosphere->GetAtmosphereTransmitanceOnGroundAtPlanetTop(Sky->Sun));

	//if it isn't visible there isn't a purpose to interpolating its location
	if (!BounceLight->IsVisible()) {
		BounceLight->SetWorldLocation(NewLocation);
	}
}

TArray<ULightComponent*> AEnvironmentBounceLight::GetLightComponents() const { return { BounceLight }; }

/*
		const ASky* Sky = Cast<ASky>(UGameplayStatics::GetActorOfClass(this, ASky::StaticClass()));
		if (!Sky) return;
		//semi-refresh because the traces won't work in the intended way here
		BounceLight->SetIntensity((Sky->IsNight() ? GetNightIntensity() : GetDayIntensity()) / 100.f * (GetIntensityCurve() ? GetIntensityCurve()->GetFloatValue(Sky->GetSunHeight()) : 1.f));
		BounceLight->SetTemperature(Sky->IsNight() ? GetNightTemperature() : GetDayTemperature());
		BounceLight->SetLightColor(Sky->IsNight() ? Sky->Moon->GetLightColor() : Sky->SkyAtmosphere->GetAtmosphereTransmitanceOnGroundAtPlanetTop(Sky->Sun));
		BounceLight->SetWorldLocation(GetActorLocation());
		BounceLight->SetVisibility(BounceLight->Intensity > 0.f);
 */

void AEnvironmentBounceLight::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(AEnvironmentBounceLight, IgnoredActors) ||
		PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(AEnvironmentBounceLight, DrawDebug) ||
		PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(AEnvironmentBounceLight, BounceSeparate) ||
		PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(AEnvironmentBounceLight, NormalBounceSeparate) ||
		PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(AEnvironmentBounceLight, BounceStartDistance) ||
		PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(AEnvironmentBounceLight, BounceEndDistance)) {
		RefreshLight(0.f, SKY);
	}
	
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
