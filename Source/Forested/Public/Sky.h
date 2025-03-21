#pragma once

#include "Forested/ForestedMinimal.h"
#include "Serialization/ObjectSaveGame.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h" 
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Sky.generated.h"

class UMaterialInstanceDynamic;
class UCurveLinearColor;
class UCurveFloat;
class UDirectionalLightComponent;
class UVolumetricCloudComponent;
class UArrowComponent;

DECLARE_DELEGATE_TwoParams(FTimedEventDelegate, float, float);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTimedEventDynamicDelegate, float, TimeInSeconds, float, ExecuteTime);
DECLARE_DELEGATE(FFrameDelayDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFrameDelayDynamicDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkyUpdate, float, DeltaTime, const ASky*, Sky);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewDay, int, Day);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewYear, int, Year);

UENUM(BlueprintType)
enum class ESeason : uint8 {
	Spring UMETA(DisplayName = "Spring"),
	Summer UMETA(DisplayName = "Summer"),
	Autumn UMETA(DisplayName = "Autumn"),
	Winter UMETA(DisplayName = "Winter")
};

struct FTimedEvents {
	TDelegateWrapper<FTimedEventDelegate> Completed = {};
	TDelegateWrapper<FTimedEventDelegate> Update = {};
	float ExecuteTime = 0.f;
	bool Pending = false;

	FORCEINLINE void EventCompleted(const float TimeInSeconds) const {
		Completed->Execute(TimeInSeconds, ExecuteTime);
	}

	FORCEINLINE void EventUpdate(const float TimeInSeconds) const {
		Update->Execute(TimeInSeconds, ExecuteTime);
	}

	//order by execution time
	FORCEINLINE bool operator<(const FTimedEvents& Event) const {
		return ExecuteTime < Event.ExecuteTime;
	}
};

UCLASS(meta=(PrioritizeCategories = "Sky Time Settings Other"))
class FORESTED_API ASky : public AActor, public ISaveObjectInterface {
	GENERATED_BODY()
	
public:	
	ASky();

protected:

	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
public:	

	virtual FString GetTypeName_Implementation() const override {
		return "Sky";
	}

	/**
	* 0-0.25 is spring, 0.25-0.5 is summer, 0.5-0.75 is fall, 0.75-1 is winter
	* assume season from day + time; spring starts on March 19, 20, or 21 (78/79/80); summer starts on June 20 or 21 (171/172); fall starts on September 22 or 23 (265/266); winter starts on December 21 or 22 (355/356);  (roughly 90 days per season)
	**/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sky")
	float GetSeasonTime() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sky")
	FORCEINLINE UMaterialParameterCollection* GetSkyCollection() const { return SkyCollection; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sky")
	FORCEINLINE ESeason GetSeason() const {
		return ESeason::Summer; //TODO: static_cast<ESeason>(GetSeasonTime() * 4.f);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sky")
	FORCEINLINE	float GetTime() const { return Time; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sky")
	FORCEINLINE	int GetDay() const { return Day; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sky")
	FORCEINLINE	int GetYear() const { return Year; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sky")
	FORCEINLINE	bool IsNight() const {
		return (TimeScale ? TimeScale->GetFloatValue(Time) : Time) > 0.5f;//GetSunHeight() < 0.0;
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sky")
	FORCEINLINE	bool IsRaining() const {
		return GetRainValue() > 0.f;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sky")
	FORCEINLINE	float GetRainValue() const {
		return RainValue;
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sky")
	FORCEINLINE float GetLightHeight() const {
		return IsNight() ? GetMoonHeight() : GetSunHeight();
	}

	//0.f at sunset, 1.f at noon, and -1.f at midnight
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sky")
	FORCEINLINE float GetSunHeight() const {
		return Sun ? FMath::GetMappedRangeValueUnclamped(FVector2D(0.f, -90.f), FVector2D(0.f, 1.f), Sun->GetComponentRotation().Pitch) : 0.f;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sky")
	FORCEINLINE float GetMoonHeight() const {
		return Moon ? FMath::GetMappedRangeValueUnclamped(FVector2D(0.f, -90.f), FVector2D(0.f, 1.f), Moon->GetComponentRotation().Pitch) : 0.f;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sky")
	FORCEINLINE	FVector GetLightDirection() const {
		return IsNight() ? GetMoonDirection() : GetSunDirection();
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sky")
	FORCEINLINE	FVector GetSunDirection() const {
		return Sun ? Sun->GetDirection() : FVector();
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sky")
	FORCEINLINE FVector GetMoonDirection() const {
		return Moon ? Moon->GetDirection() : FVector();
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sky")
	FORCEINLINE float GetTimePassed() const {
		return Time + Day + Year * 365;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sky")
	FORCEINLINE float GetTimePassedInSeconds() const {
		return GetDaytimePassedInSeconds() + GetNighttimePassedInSeconds();
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sky")
	FORCEINLINE	float GetDaytime() const {
		return FMath::Clamp(Time * 2.f, 0.f, 1.f);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sky")
	FORCEINLINE float GetDaytimePassed() const {
		return GetDaytime() + Day + Year * 365;
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sky")
	FORCEINLINE float GetDaytimePassedInSeconds() const {
		return GetDaytimePassed() * DayTimeSpeedInSecondsPerDay;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sky")
	FORCEINLINE	float GetNighttime() const {
		return FMath::Clamp((Time - 0.5f) * 2.f, 0.f, 1.f);
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sky")
	FORCEINLINE float GetNighttimePassed() const {
		return GetNighttime() + Day + Year * 365;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sky")
	FORCEINLINE float GetNighttimePassedInSeconds() const {
		return GetNighttimePassed() * NightTimeSpeedInSecondsPerNight;
	}

	FORCEINLINE bool HasBeenTickedThisFrame() const { return LastTickedFrame == GFrameCounter; }

	virtual void Tick(float DeltaTime) override;

	virtual void OnConstruction(const FTransform& ActorTransform) override;

	UFUNCTION(BlueprintCallable, Category = "Sky")
	void GetWindDirectionAndSpeed(FVector& WindDirection, float& WindSpeed) const;
	
	UFUNCTION(BlueprintCallable, Category = "Sky")
	void RenderSky(float DeltaTime);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Render Sky", Category = "Sky")
	void ReceiveRenderSky(float DeltaTime, ASky* Sky);

	UFUNCTION()
	void SetRainValue(float InRainValue);

	UPROPERTY(BlueprintAssignable)
	FOnNewDay OnNewDay;

	UPROPERTY(BlueprintAssignable)
	FOnNewYear OnNewYear;

	UPROPERTY(BlueprintAssignable)
	FOnSkyUpdate OnSkyUpdate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sky")
	UStaticMesh* SkyStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
	UMaterialInterface* SkyMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
	UMaterialInterface* VolumetricCloudMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky|Sun")
	UCurveLinearColor* SunShadowSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky|Sun", meta = (UIMin = "0.0", UIMax = "1.0"))
	float SunDiskBrightness = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sky|Moon")
	UStaticMesh* MoonStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky|Moon")
	UMaterialInterface* MoonMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky|Moon", meta = (UIMin = "0.0", UIMax = "1.0"))
	float MoonDiskBrightness = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky|Moon")
	float MoonDistance = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Sky")
	float GalaxyBrightness = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Sky")
	float StarBrightness = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Sky", meta = (UIMin = "0.0", UIMax = "1.0"))
	float ShadowBiasOverride = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Sky", meta = (UIMin = "0.0", UIMax = "1.0"))
	float ShadowSlopeBiasOverride = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Sky", meta = (UIMin = "0.0", UIMax = "1.0"))
	float ShadowSharpenOverride = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Sky", meta = (UIMin = "0.0", UIMax = "4.0"))
	int ShadowCascadesOverride = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Time", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float Time = 0.f;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time", meta = (ClampMin = "-90.0", ClampMax = "90.0", UIMin = "-90.0", UIMax = "90.0"))
	float Latitude = 25.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Time", meta = (ClampMin = "1.0", ClampMax = "365.0", UIMin = "1.0", UIMax = "365.0"))
	int Day = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Time", meta = (ClampMin = "0.0", UIMin = "0.0"))
	int Year = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Time|Rain", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float RainValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	float DayTimeSpeedInSecondsPerDay = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	float NightTimeSpeedInSecondsPerNight = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time")
	UCurveFloat* TimeScale;

	UPROPERTY()
	UMaterialParameterCollection* SkyCollection;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time|Rain")
	FLinearColor ClearRayleighScattering = FLinearColor(0.175287f, 0.409607f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time|Rain")
	FLinearColor RainRayleighScattering = FLinearColor(0.4f, 0.4f, 0.4f);

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Mesh")
	UArrowComponent* NorthDirection;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Mesh")
	UDirectionalLightComponent* Sun;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Mesh")
	UDirectionalLightComponent* Moon;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Mesh")
	USceneComponent* EarthLocationComponent;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Mesh")
	USceneComponent* EarthMovementComponent;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Mesh")
	USceneComponent* MoonMovementComponent;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Mesh")
	UStaticMeshComponent* SkyMesh;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Mesh")
	UStaticMeshComponent* MoonMesh;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Mesh")
	UVolumetricCloudComponent* VolumetricCloud;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Mesh")
	USkyLightComponent* SkyLight;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Mesh")
	USkyAtmosphereComponent* SkyAtmosphere;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Mesh")
	UExponentialHeightFogComponent* ExponentialHeightFog;

	/*
	 * adds a timed event based on in-game time
	 * a timed events fires once upon completion
	 * meaning this will wait on in-game pause menus and other things which pause the execution of the sky loop
	 */
	void AddTimedEvent(const float ExecuteTime = 0.f, const TDelegateWrapper<FTimedEventDelegate>& Complete = {});

	/*
	 * adds a timer based on in-game time
	 * a timer repeats every frame until completion
	 * meaning this will wait on in-game pause menus and other things which pause the execution of the sky loop
	 */
	void AddTimer(const float ExecuteTime = 0.f, const TDelegateWrapper<FTimedEventDelegate>& Complete = {}, const TDelegateWrapper<FTimedEventDelegate>& Update = {});

	/*
	 * adds a delay by a single frame
	 * meaning this will wait on in-game pause menus and other things which pause the execution of the sky loop
	 */
	void AddFrameDelay(const TDelegateWrapper<FFrameDelayDelegate>& Complete = {});
	
protected:

	UPROPERTY(BlueprintReadOnly, Category = "Mesh")
	UMaterialInstanceDynamic* SkyDynamicMaterial;

	UPROPERTY(BlueprintReadOnly, Category = "Mesh")
	UMaterialInstanceDynamic* MoonDynamicMaterial;

private:
	
	TArray<FTimedEvents> Timers;
	
	TArray<FTimedEvents> TimedEvents;

	TArray<FFrameDelayDelegate> FrameDelays;
	
	TArray<FFrameDelayDelegate> PendingFrameDelays;

	// An internally consistent clock, independent of World.  Advances during ticking.
	double InternalTime = 0.0;
	
	// Set this to GFrameCounter when Timer is ticked. To figure out if Timer has been already ticked or not this frame.
	uint64 LastTickedFrame = static_cast<uint64>(-1);

	int PendingCounter = 0;
	
	int RegCounter = 0;
};
//Timer Async Event Action


UCLASS()
class FORESTED_API UTimedEventAsyncAction : public UBlueprintAsyncActionBase {
	GENERATED_BODY()

protected:

	/*
	 * parameters
	 */

	ASky* Sky;

	float ExecuteTime;

	UPROPERTY(BlueprintAssignable)
	FTimedEventDynamicDelegate Complete;

	/*
	 * functions
	 */

	//Adds a timed event, fires once upon completion
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly = "true"), Category = "Timed Events")
	static UTimedEventAsyncAction* AddTimedEvent(const float ExecuteTime = 0.f);

	virtual void Activate() override;
};

UCLASS()
class FORESTED_API UTimerAsyncAction : public UBlueprintAsyncActionBase {
	GENERATED_BODY()

protected:

	/*
	 * parameters
	 */

	ASky* Sky;

	float ExecuteTime;

	UPROPERTY(BlueprintAssignable)
	FTimedEventDynamicDelegate Update;

	UPROPERTY(BlueprintAssignable)
	FTimedEventDynamicDelegate Complete;

	/*
	 * functions
	 */
	
	//Add a timer, this fires every frame until completed
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly = "true"), Category = "Timed Events")
	static UTimerAsyncAction* AddTimer(const float ExecuteTime = 0.f);

	virtual void Activate() override;
};

UCLASS()
class FORESTED_API UFrameDelayAsyncAction : public UBlueprintAsyncActionBase {
	GENERATED_BODY()

protected:

	/*
	 * parameters
	 */

	ASky* Sky;

	UPROPERTY(BlueprintAssignable)
	FFrameDelayDynamicDelegate Complete;

	/*
	 * functions
	 */
	
	//Add a frame delay, fires after one frame
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly = "true"), Category = "Timed Events")
	static UFrameDelayAsyncAction* AddFrameDelay();

	virtual void Activate() override;
};