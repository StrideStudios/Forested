#pragma once

#include "Components/LightComponent.h"
#include "Forested/ForestedMinimal.h"
#include "GameFramework/Actor.h"
#include "EnvironmentLight.generated.h"

class UArrowComponent;
class ASky;
class UPointLightComponent;
class URectLightComponent;

UCLASS(Abstract, MinimalAPI, meta=(PrioritizeCategories = "Base"))
class AEnvironmentLight : public AActor {
	GENERATED_BODY()
	
public:	
	AEnvironmentLight();

protected:

	virtual void BeginPlay() override;
	
public:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USceneComponent* Root;
	
	UFUNCTION(BlueprintCallable, Category = "Environment Light")
	virtual void RefreshLight(float DeltaTime, const ASky* Sky);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Refresh Light")
	void ReceiveRefreshLight(float DeltaTime, const ASky* Sky);

	virtual TArray<ULightComponent*> GetLightComponents() const {
		return {};
	}

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	/*
	 * Getters
	 */

	UFUNCTION(BlueprintGetter, Category = "Environment Light|Intensity")
	UCurveFloat* GetIntensityCurve() const { return IntensityCurve; }

	UFUNCTION(BlueprintGetter, Category = "Environment Light|Visibility")
	bool IsEnabled() const { return bEnabled; }

	UFUNCTION(BlueprintGetter, Category = "Environment Light|Intensity")
	float GetDayIntensity() const { return DayIntensity; }

	UFUNCTION(BlueprintGetter, Category = "Environment Light|Intensity")
	float GetNightIntensity() const { return NightIntensity; }

	UFUNCTION(BlueprintGetter, Category = "Environment Light|Temperature")
	float GetDayTemperature() const { return DayTemperature; }
	
	UFUNCTION(BlueprintGetter, Category = "Environment Light|Temperature")
	float GetNightTemperature() const { return NightTemperature; }

	/*
	 * Setters
	 */

	UFUNCTION(BlueprintCallable, Category = "Environment Light|Visibility")
	void Enable();

	UFUNCTION(BlueprintCallable, Category = "Environment Light|Visibility")
	void Disable();

	UFUNCTION(BlueprintSetter, Category = "Environment Light|Visibility")
	void SetEnabled(const bool InEnabled) {
		if (InEnabled) {
			Enable();
			return;
		}
		Disable();
	}

	UFUNCTION(BlueprintSetter, Category = "Environment Light|Intensity")
	void SetDayIntensity(const float InDayIntensity) {
		DayIntensity = InDayIntensity;
		RefreshLight(0.f, SKY);
	}
	
	UFUNCTION(BlueprintSetter, Category = "Environment Light|Intensity")
	void SetNightIntensity(const float InNightIntensity) {
		NightIntensity = InNightIntensity;
		RefreshLight(0.f, SKY);
	}

	UFUNCTION(BlueprintSetter, Category = "Environment Light|Intensity")
	void SetDayTemperature(const float InDayTemperature) {
		DayTemperature = InDayTemperature;
		RefreshLight(0.f, SKY);
	}

	UFUNCTION(BlueprintSetter, Category = "Environment Light|Intensity")
	void SetNightTemperature(const float InNightTemperature) {
		NightTemperature = InNightTemperature;
		RefreshLight(0.f, SKY);
	}
	
private:

	UPROPERTY(EditDefaultsOnly, Category = "Base")
	UCurveFloat* IntensityCurve = nullptr;

	UPROPERTY(EditAnywhere, Category = "Base")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, Category = "Base")
	float DayIntensity = 20.f;

	UPROPERTY(EditAnywhere, Category = "Base")
	float NightIntensity = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Base")
	float DayTemperature = 5500.f;

	UPROPERTY(EditAnywhere, Category = "Base")
	float NightTemperature = 12000.f;

};

UCLASS(meta=(PrioritizeCategories = "Point"))
class FORESTED_API AEnvironmentPointLight : public AEnvironmentLight {
	GENERATED_BODY()

public:
	
	AEnvironmentPointLight();

	virtual TArray<ULightComponent*> GetLightComponents() const override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Point")
	UPointLightComponent* PointLight;
	
};

UCLASS(meta=(PrioritizeCategories = "Rect"))
class FORESTED_API AEnvironmentRectLight : public AEnvironmentLight {
	GENERATED_BODY()

public:
	
	AEnvironmentRectLight();

	virtual TArray<ULightComponent*> GetLightComponents() const override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rect")
	URectLightComponent* RectLight;
	
};

UCLASS(meta=(PrioritizeCategories = "Bounce"))
class FORESTED_API AEnvironmentBounceLight : public AEnvironmentLight {
	GENERATED_BODY()

public:
	
	AEnvironmentBounceLight();

protected:

	virtual void BeginPlay() override;
	
public:
	
	virtual void RefreshLight(float DeltaTime, const ASky* Sky) override;
	
	virtual TArray<ULightComponent*> GetLightComponents() const override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bounce")
	UPointLightComponent* BounceLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bounce")
	UArrowComponent* ForwardDirection;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bounce")
	UArrowComponent* LightDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce")
	TArray<AActor*> IgnoredActors = {};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce")
	bool DrawDebug = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce")
	float BounceSeparate = 35.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce")
	float NormalBounceSeparate = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce")
	float BounceStartDistance = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounce")
	float BounceEndDistance = 1000.f;

private:
	
	FVector NewLocation;

	float NewIntensity;
};