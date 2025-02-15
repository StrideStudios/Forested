#include "Sky.h"
#include "ForestedLibrary.h"
#include "FPlayer.h"
#include "Components/VolumetricCloudComponent.h" 
#include "Components/ArrowComponent.h" 
#include "Kismet/GameplayStatics.h"
#include "Curves/CurveLinearColor.h" 
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "UObject/ConstructorHelpers.h"

ASky::ASky(){
	PrimaryActorTick.bCanEverTick = true;
	
	SetActorEnableCollision(false);
	EarthLocationComponent = CreateDefaultSubobject<USceneComponent>(TEXT("EarthLocationComponent"));
	RootComponent = EarthLocationComponent;
	EarthMovementComponent = CreateDefaultSubobject<USceneComponent>(TEXT("EarthMovementComponent"));
	EarthMovementComponent->SetupAttachment(RootComponent);
	SkyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SkyMesh"));
	SkyMesh->SetRelativeScale3D(FVector(6371.f));
	SkyMesh->SetCastShadow(false);
	SkyMesh->bCastDynamicShadow = false;
	SkyMesh->SetupAttachment(EarthMovementComponent);
	MoonMovementComponent = CreateDefaultSubobject<USceneComponent>(TEXT("MoonMovementComponent"));
	MoonMovementComponent->SetupAttachment(EarthMovementComponent);
	MoonMovementComponent->SetRelativeRotation(FRotator(0.f, 5.145f, 0.f));
	MoonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Moon Mesh"));
	MoonMesh->SetupAttachment(MoonMovementComponent);
	Sun = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Sun"));
	Sun->SetupAttachment(RootComponent);
	Sun->ForwardShadingPriority = 1;
	Moon = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Moon"));
	Moon->SetupAttachment(MoonMovementComponent);
	VolumetricCloud = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("Volumetric Cloud"));
	VolumetricCloud->SetupAttachment(RootComponent);
	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("Sky Light"));
	SkyLight->SetupAttachment(RootComponent);
	SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("Sky Atmosphere"));
	SkyAtmosphere->SetupAttachment(RootComponent);
	ExponentialHeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("Exponential Height Fog"));
	ExponentialHeightFog->SetupAttachment(RootComponent);
	NorthDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("North Direction"));
	NorthDirection->SetupAttachment(RootComponent);
	NorthDirection->ArrowSize = 1.f;

	const ConstructorHelpers::FObjectFinder<UMaterialParameterCollection> ParameterCollection(TEXT("/Game/Geometry/ParameterCollections/MPC_Sky.MPC_Sky"));
	if (ParameterCollection.Succeeded()) {
		SkyCollection = ParameterCollection.Object;
	}
}

void ASky::BeginPlay() {
	Super::BeginPlay();
	
}

void ASky::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);
	TimedEvents.Empty();
}

void ASky::Tick(const float DeltaTime){
	Super::Tick(DeltaTime);
	if (!PLAYER || !PLAYER->IsGameStarted()) return;
	const double TimeLerp = UKismetMathLibrary::MapRangeClamped(GetSunHeight(), -0.1f, -0.05f, 0.f, 1.f);
	const double TimeToAdd = DeltaTime * (1.f / (2.f * FMath::Lerp(NightTimeSpeedInSecondsPerNight, DayTimeSpeedInSecondsPerDay, TimeLerp)));
	Time += TimeToAdd;
	if (Time >= 1.f) {
		Time = 0.f;
		++Day;
		if (Day > 365) {
			Day = 0;
			++Year;
			OnNewYear.Broadcast(Year);
		}
		OnNewDay.Broadcast(Day);
	}
	
	InternalTime += DeltaTime;
	
	while (TimedEvents.Num()) {
		FTimedEvents& Top = TimedEvents.HeapTop();
		if (InternalTime < Top.ExecuteTime)
			break;
		Top.EventCompleted(InternalTime);
		TimedEvents.HeapPop(Top);
	}

	while (Timers.Num()) {
		FTimedEvents& Top = Timers.HeapTop();
		if (InternalTime < Top.ExecuteTime)
			break;
		Top.EventCompleted(InternalTime);
		Timers.HeapPop(Top);
	}

	for (FTimedEvents& TimedEvent : Timers)
		TimedEvent.EventUpdate(InternalTime);

	//execute all frame delays and empty the set
	for (FFrameDelayDelegate FrameDelay : FrameDelays) {
		FrameDelay.ExecuteIfBound();
		RegCounter++;
	}

	FrameDelays.Empty();

	//add pending delays to no longer be pending and empty the pending set
	for (FFrameDelayDelegate PendingFrameDelay : PendingFrameDelays) {
		FrameDelays.Emplace(PendingFrameDelay);
	}

	PendingFrameDelays.Empty();

	LastTickedFrame = GFrameCounter;
	
	RenderSky(DeltaTime);
}

void ASky::OnConstruction(const FTransform& ActorTransform){
	SkyDynamicMaterial = UMaterialInstanceDynamic::Create(SkyMaterial, this);
	MoonDynamicMaterial = UMaterialInstanceDynamic::Create(MoonMaterial, this);
	SkyMesh->SetStaticMesh(SkyStaticMesh);
	SkyMesh->SetMaterial(0, SkyDynamicMaterial);
	MoonMesh->SetStaticMesh(MoonStaticMesh);
	MoonMesh->SetMaterial(0, MoonDynamicMaterial);
	VolumetricCloud->SetMaterial(VolumetricCloudMaterial);
	RenderSky(0.f);
}

void ASky::GetWindDirectionAndSpeed(FVector& WindDirection, float& WindSpeed) const {
	if (!SkyCollection || !GetWorld()) return;
	const UMaterialParameterCollectionInstance* SkyCollectionInstance = GetWorld()->GetParameterCollectionInstance(SkyCollection);
	FLinearColor OutWindDirection;
	SkyCollectionInstance->GetVectorParameterValue("WindDirection", OutWindDirection);
	WindDirection = FVector(OutWindDirection);
	SkyCollectionInstance->GetScalarParameterValue("WindSpeed", WindSpeed);
}

void ASky::RenderSky(const float DeltaTime) {
	
	/* ----- Fog Settings -----
	 * More dense fog closer to sunset and during rain
	 */
	
	SkyAtmosphere->SetRayleighScattering(FMath::Lerp(ClearRayleighScattering, RainRayleighScattering, GetRainValue()));
	ExponentialHeightFog->SetFogDensity(UKismetMathLibrary::Ease(0.006f, 0.004f, FMath::Clamp(GetSunHeight(), 0.f, 1.f), EEasingFunc::EaseOut, 4.f) + UKismetMathLibrary::Ease(0.f, 0.01f, GetRainValue(), EEasingFunc::EaseIn, 6.f));
	ExponentialHeightFog->SecondFogData.FogDensity = UKismetMathLibrary::Ease(0.005f, 0.01f, FMath::Abs(GetSunHeight()), EEasingFunc::EaseOut, 4.f);
	ExponentialHeightFog->SetVolumetricFogExtinctionScale(FMath::Lerp(
		UKismetMathLibrary::Ease(0.5f, 1.f, FMath::Clamp(GetSunHeight() * -1.f, 0.f, 1.f), EEasingFunc::EaseIn, 0.75f),
		UKismetMathLibrary::Ease(1.5f, 1.f, FMath::Clamp(GetSunHeight(), 0.f, 1.f), EEasingFunc::EaseIn, 0.75f),
		UKismetMathLibrary::MapRangeClamped(GetSunHeight(), 0.f, 0.075f, 0.f, 1.f))
		);
	ExponentialHeightFog->SetFogHeightFalloff(UKismetMathLibrary::Ease(0.75f, 0.25f, FMath::Clamp(GetSunHeight(), 0.f, 1.f), EEasingFunc::EaseOut, 4.f));

	/* ----- Sun Settings -----
	 * Less Intense and Cooler Color during rain
	 * some functions will lower intensity while keeping its effect on the sky
	 */
	
	Sun->SetIntensity(UKismetMathLibrary::Ease(10.f, 2.f, GetRainValue(), EEasingFunc::EaseIn, 2.f));
	Sun->SetTemperature(FMath::Lerp(5500.f, 6500.f, GetRainValue()));
	Sun->SetBloomScale(IsNight() ? 0.125f: UKismetMathLibrary::FInterpTo(Sun->BloomScale, 0.125f/*SeeLight ? 0.125f : 0.f*/, DeltaTime, 0.1f));
	Sun->SetLightFunctionDisabledBrightness(IsNight() ? 0.f : UKismetMathLibrary::FInterpTo(Sun->DisabledBrightness, 1.f/*see light ? 1.f : 0.f*/, DeltaTime, 0.1f));
	Sun->SetOcclusionMaskDarkness(IsNight() ? 1.f : UKismetMathLibrary::FInterpTo(Sun->OcclusionMaskDarkness, 0.05f/*SeeLight ? 0.05f : 1.f*/, DeltaTime, 0.1f));
	const bool bSunEnabled = Sun->DisabledBrightness > 0.f && !IsNight();
	Sun->SetLightingChannels(bSunEnabled, false, false);
	Sun->SetCastShadows(bSunEnabled);
	Sun->AtmosphereSunDiskColorScale = FLinearColor(FVector(SunDiskBrightness));

	if (SunShadowSettings) {
		const float SunHeight = GetSunHeight();
		Sun->SetShadowBias(ShadowBiasOverride != 0.f ? ShadowBiasOverride : SunShadowSettings->GetLinearColorValue(SunHeight).R);
		Sun->SetShadowSlopeBias(ShadowSlopeBiasOverride != 0.f ? ShadowSlopeBiasOverride : SunShadowSettings->GetLinearColorValue(SunHeight).G);
		Sun->ShadowSharpen = ShadowSharpenOverride != 0.f ? ShadowSharpenOverride : SunShadowSettings->GetLinearColorValue(SunHeight).B;
		Sun->SetDynamicShadowCascades(ShadowCascadesOverride != 0 ? ShadowCascadesOverride : FMath::FloorToInt(SunShadowSettings->GetLinearColorValue(SunHeight).A));
	}

	const double ScaledTime = TimeScale ? TimeScale->GetFloatValue(Time) : Time;
	const double LatitudeRad = FMath::DegreesToRadians(Latitude);
	const double YearDegree = (Day + ScaledTime) * 360.0 / 366.0;
	const double YearDegreeRad = FMath::DegreesToRadians(YearDegree);
	
	/* ----- Moon Settings -----
	 * Less Intense when low in the sky and when sun isn't pointing directly at it
	 */
	
	const float MoonDirectionalIntensity = FMath::Pow(UKismetMathLibrary::MapRangeClamped(UKismetMathLibrary::Dot_VectorVector(GetSunDirection(), GetMoonDirection()) * -1.f, -1.f, 1.f, 0.f, 1.f), 4.f);
	const float MoonHeightIntensity = UKismetMathLibrary::MapRangeClamped(FMath::Abs(GetMoonHeight()), 0.05f, 0.5f, 0.f, 1.f);
	Moon->SetIntensity(UKismetMathLibrary::Ease(0.25f, 0.05f, GetRainValue(), EEasingFunc::EaseIn, 2.f) * MoonDirectionalIntensity * MoonHeightIntensity);
	Moon->SetTemperature(FMath::Lerp(10000.f, 11000.f, GetRainValue()));
	Moon->SetBloomScale(IsNight() ? UKismetMathLibrary::FInterpTo(Moon->BloomScale, 0.125f/*SeeLight ? 0.125f : 0.f*/, DeltaTime, 0.1f) : 0.f);

	const float MoonColor = 1.f / MoonDirectionalIntensity * 0.0075f;
	Moon->AtmosphereSunDiskColorScale = FLinearColor(FVector(MoonColor));
	Moon->SetLightFunctionDisabledBrightness(IsNight() ? UKismetMathLibrary::FInterpTo_Constant(Moon->DisabledBrightness, 1.f/*SeeLight ? UKismetMathLibrary::MapRangeClamped(GetSunHeight(), -0.1f, 0.f, 1.f, 0.f) : 0.f*/, DeltaTime, 0.1f) : 0.f);
	Moon->SetOcclusionMaskDarkness(IsNight() ? UKismetMathLibrary::FInterpTo(Moon->OcclusionMaskDarkness, 0.05f/*SeeLight ? 0.05f : 1.f*/, DeltaTime, 0.1f) : 1.f);
	const bool bMoonEnabled = Moon->DisabledBrightness > 0.f && IsNight();
	Moon->SetLightingChannels(bMoonEnabled, false, false);
	Moon->SetCastShadows(bMoonEnabled);
	MoonMesh->SetWorldLocation(Moon->GetDirection()*-MoonDistance);
	MoonMesh->SetWorldRotation(FRotator(Moon->GetDirection().ToOrientationRotator().Pitch, Moon->GetDirection().ToOrientationRotator().Yaw,Moon->GetComponentRotation().Roll+90.f));
	Moon->SetRelativeRotation(FRotator((Day + Time) / 27.321528f * 360.f, 90.f, 0.f));
	Moon->ForwardShadingPriority = IsNight() ? 2 : 0;
	Moon->AtmosphereSunDiskColorScale = FLinearColor(FVector(MoonDiskBrightness));

	//tilt the moon throughout the year
	const float MoonComponentPitch = FMath::Cos(YearDegreeRad) * 5.145002f;
	const float MoonComponentYaw = FMath::Sin(YearDegreeRad) * 5.145002f;

	MoonMovementComponent->SetRelativeRotation(FRotator(MoonComponentPitch, MoonComponentYaw, 0.f));
	
	/* ----- Earth Movement -----
	 * The sky and moon move which gives the appearance of Earth moving
	 */
	
	const double Declination = FMath::DegreesToRadians(23.45 * FMath::Sin(FMath::DegreesToRadians(360.0/365.0 * (Day + ScaledTime - 81))));
	const double HalfDayLength =  FMath::Acos(-FMath::Tan(LatitudeRad)*FMath::Tan(Declination)) / (2.f * PI);
	const double AdjustedDayTime = UKismetMathLibrary::MapRangeClamped(ScaledTime, 0.f, 0.5f, 0.5f - HalfDayLength, 0.5f + HalfDayLength);
	const double AdjustedNightTime = FMath::Fmod(UKismetMathLibrary::MapRangeClamped(ScaledTime, 0.5f, 1.f, 0.5f + HalfDayLength, 0.5f - HalfDayLength + 1.f), 1.f);
	const double AdjustedTime = IsNight() ? AdjustedNightTime : AdjustedDayTime;
	const double HourAngle = FMath::Fmod(AdjustedTime * 360.f, 360.f) - 180.f;
	const double HourAngleRad = FMath::DegreesToRadians(HourAngle);
	const double Elevation = FMath::Asin(FMath::Sin(Declination)*FMath::Sin(LatitudeRad) + FMath::Cos(Declination)*FMath::Cos(LatitudeRad)*FMath::Cos(HourAngleRad));
	const double ElevationDeg = 180.f + FMath::RadiansToDegrees(Elevation);
	const double PreAzimuthRad = FMath::Acos((FMath::Sin(Declination)*FMath::Cos(LatitudeRad) - FMath::Cos(Declination)*FMath::Sin(LatitudeRad)*FMath::Cos(HourAngleRad)) / FMath::Cos(Elevation));
	const double PreAzimuthDeg = 180.f - FMath::RadiansToDegrees(PreAzimuthRad);
	const double Azimuth = HourAngle > 0.f ?  FMath::Fmod(PreAzimuthDeg + 180.f, 360.f) : FMath::Fmod(540.f - PreAzimuthDeg, 360.f);
	
	Sun->SetRelativeRotation(FRotator(ElevationDeg, Azimuth, 0.f));
	
	const double MaxAdjustedDayTime = UKismetMathLibrary::MapRangeClamped(ScaledTime, 0.f, 0.5f, 0.25f, 0.75f);
	const double MaxAdjustedNightTime = FMath::Fmod(UKismetMathLibrary::MapRangeClamped(ScaledTime, 0.5f, 1.f, 0.75f, 1.25f), 1.f);
	const double MaxAdjustedTime = IsNight() ? MaxAdjustedNightTime : MaxAdjustedDayTime;
	const double MaxHourAngle = FMath::Fmod(MaxAdjustedTime * 360.f, 360.f) - 180.f;
	
	EarthMovementComponent->SetRelativeRotation(FRotator(-Latitude, 180.f, MaxHourAngle + YearDegree));

	SkyDynamicMaterial->SetVectorParameterValue(TEXT("RotateAxis"), FLinearColor(FVector4(Sun->GetRightVector(), 0.f)));
	SkyDynamicMaterial->SetScalarParameterValue(TEXT("SunBrightness"), Sun->Intensity);
	SkyDynamicMaterial->SetScalarParameterValue(TEXT("SunHeight"), GetSunHeight());
	SkyDynamicMaterial->SetScalarParameterValue(TEXT("GalaxyBrightness"), GalaxyBrightness);
	SkyDynamicMaterial->SetScalarParameterValue(TEXT("StarBrightness"), StarBrightness);
	MoonDynamicMaterial->SetVectorParameterValue(TEXT("SunVector"), GetSunDirection());

	if (SkyCollection && GetWorld()) {
		UMaterialParameterCollectionInstance* SkyCollectionInstance = GetWorld()->GetParameterCollectionInstance(SkyCollection);
		SkyCollectionInstance->SetScalarParameterValue("SunHeight", GetSunHeight());
		SkyCollectionInstance->SetScalarParameterValue("SkyTime", GetTimePassed());
		SkyCollectionInstance->SetScalarParameterValue("DayTime", GetDaytimePassed());
		SkyCollectionInstance->SetScalarParameterValue("NightTime", GetNighttimePassed());
	}
	
	this->ReceiveRenderSky(DeltaTime, this);
	OnSkyUpdate.Broadcast(DeltaTime, this);
}

void ASky::SetRainValue(const float InRainValue) {
	RainValue = InRainValue;
	if (!SkyCollection || !GetWorld()) return;
	UMaterialParameterCollectionInstance* SkyCollectionInstance = GetWorld()->GetParameterCollectionInstance(SkyCollection);
	SkyCollectionInstance->SetScalarParameterValue("RainValue", RainValue);
}

void ASky::AddTimedEvent_Internal(const FTimedEventDelegate& Complete, const float ExecuteTime) {
	if (!Complete.IsBound()) {
		LOG_ERROR("Invalid delegate given, make sure callback is valid");
		return;
	}
	if (ExecuteTime <= 0.f) {
		LOG_ERROR("Invalid time given, make execute time is greater than 0");
		return;
	}

	check(IsInGameThread());
	
	auto PushEvent = [this, Complete, ExecuteTime] {
		FTimedEvents Event;
		Event.Completed = Complete;
		Event.Update = {};
		Event.ExecuteTime = InternalTime + ExecuteTime;
		TimedEvents.HeapPush(Event);
	};

	//either set push event to happen in a frame or call it immediately
	if (HasBeenTickedThisFrame()) {
		PushEvent();
		return;
	}
	AddFrameDelay(PushEvent);
}

void ASky::AddTimer_Internal(const FTimedEventDelegate& Complete, const FTimedEventDelegate& Update, const float ExecuteTime) {
	if (!Complete.IsBound() || !Update.IsBound()) {
		LOG_ERROR("Invalid delegate given, make sure callback is valid");
		return;
	}
	if (ExecuteTime <= 0.f) {
		LOG_ERROR("Invalid time given, make execute time is greater than 0");
		return;
	}
	
	check(IsInGameThread());

	auto PushEvent = [this, Complete, Update, ExecuteTime] {
		FTimedEvents Event;
		Event.Completed = Complete;
		Event.Update = Update;
		Event.ExecuteTime = InternalTime + ExecuteTime;
		Timers.HeapPush(Event);
	};
	
	//either set push event to happen in a frame or call it immediately
	if (HasBeenTickedThisFrame()) {
		PushEvent();
		return;
	}
	AddFrameDelay(PushEvent);
}

void ASky::AddFrameDelay_Internal(const FFrameDelayDelegate& Complete) {
	if (!Complete.IsBound()) {
		LOG_ERROR("Invalid delegate given, make sure callback is valid");
		return;
	}
	
	check(IsInGameThread());

	if (HasBeenTickedThisFrame()) {
		FrameDelays.Emplace(Complete);
		return;
	}
	PendingCounter++;
	PendingFrameDelays.Emplace(Complete);
}

UTimedEventAsyncAction* UTimedEventAsyncAction::AddTimedEvent(const float ExecuteTime) {
	UTimedEventAsyncAction* Node = NewObject<UTimedEventAsyncAction>();
	Node->Sky = SKY;
	Node->ExecuteTime = ExecuteTime;
	return Node;
}

void UTimedEventAsyncAction::Activate() {
	Sky->AddTimedEvent(ExecuteTime, [this](const float Time, const float ExecuteTime){
		Complete.Broadcast(Time, ExecuteTime);
	});
}

UTimerAsyncAction* UTimerAsyncAction::AddTimer(const float ExecuteTime) {
	UTimerAsyncAction* Node = NewObject<UTimerAsyncAction>();
	Node->Sky = SKY;
	Node->ExecuteTime = ExecuteTime;
	return Node;
}

void UTimerAsyncAction::Activate() {
	Sky->AddTimer(ExecuteTime, [this](const float Time, const float ExecuteTime){
		Complete.Broadcast(Time, ExecuteTime);
	}, [this](const float Time, const float ExecuteTime){
		Update.Broadcast(Time, ExecuteTime);
	});
}

UFrameDelayAsyncAction* UFrameDelayAsyncAction::AddFrameDelay() {
	UFrameDelayAsyncAction* Node = NewObject<UFrameDelayAsyncAction>();
	Node->Sky = SKY;
	return Node;
}

void UFrameDelayAsyncAction::Activate() {
	Sky->AddFrameDelay([this] {
		Complete.Broadcast();
	});
}


