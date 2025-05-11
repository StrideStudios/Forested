#pragma once

#include "Forested/ForestedMinimal.h"
#include "GameFramework/Actor.h"
#include "TimedLight.generated.h"

class ASky;
class USpotLightComponent;
class UPointLightComponent;
class URectLightComponent;

UCLASS(Abstract, MinimalAPI, meta=(PrioritizeCategories = "Base"))
class ATimedLight : public AActor {
	GENERATED_BODY()
	
public:	
	ATimedLight();

protected:

	virtual void BeginPlay() override;
	
public:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USceneComponent* Root;
	
	UFUNCTION(BlueprintCallable, Category = "Timed Light")
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

	UFUNCTION(BlueprintGetter, Category = "Timed Light|Intensity")
	UCurveFloat* GetIntensityCurve() const { return IntensityCurve; }

	UFUNCTION(BlueprintGetter, Category = "Timed Light|Visibility")
	bool IsEnabled() const { return bEnabled; }

	UFUNCTION(BlueprintGetter, Category = "Timed Light|Visibility")
	float GetStartTime() const { return StartTime; }

	UFUNCTION(BlueprintGetter, Category = "Timed Light|Visibility")
	float GetEndTime() const { return EndTime; }

	/*
	 * Setters
	 */

	UFUNCTION(BlueprintCallable, Category = "Timed Light|Visibility")
	void Enable();

	UFUNCTION(BlueprintCallable, Category = "Timed Light|Visibility")
	void Disable();

	UFUNCTION(BlueprintSetter, Category = "Timed Light|Visibility")
	void SetEnabled(const bool InEnabled) {
		if (InEnabled) {
			Enable();
			return;
		}
		Disable();
	}

	UFUNCTION(BlueprintSetter, Category = "Timed Light|Visibility")
	void SetStartTime(const float InStartTime) {
		StartTime = InStartTime;
		RefreshLight(0.f, SKY);
	}

	UFUNCTION(BlueprintSetter, Category = "Timed Light|Visibility")
	void SetEndTime(const float InEndTime) {
		EndTime = InEndTime;
		RefreshLight(0.f, SKY);
	}
	
private:

	UPROPERTY(EditDefaultsOnly, Category = "Base")
	UCurveFloat* IntensityCurve = nullptr;

	UPROPERTY(EditAnywhere, Category = "Base")
	bool bEnabled = true;

	// The activation time required to activate this light
	UPROPERTY(EditAnywhere, Category = "Base", meta = (ClampMin = "-1", ClampMax = "1", UIMin = "-1", UIMax = "1"))
	float StartTime = -1.f;

	// The de-activation time required to activate this light
	UPROPERTY(EditAnywhere, Category = "Base", meta = (ClampMin = "-1", ClampMax = "1", UIMin = "-1", UIMax = "1"))
	float EndTime = 0.f;

};

UCLASS(meta=(PrioritizeCategories = "Point"))
class FORESTED_API ATimedPointLight : public ATimedLight {
	GENERATED_BODY()

public:
	
	ATimedPointLight();

	virtual TArray<ULightComponent*> GetLightComponents() const override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Point")
	UPointLightComponent* PointLight;
	
};

UCLASS(meta=(PrioritizeCategories = "Rect"))
class FORESTED_API ATimedRectLight : public ATimedLight {
	GENERATED_BODY()

public:
	
	ATimedRectLight();

	virtual TArray<ULightComponent*> GetLightComponents() const override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rect")
	URectLightComponent* RectLight;
	
};

UCLASS(meta=(PrioritizeCategories = "Spot"))
class FORESTED_API ATimedSpotLight : public ATimedLight {
	GENERATED_BODY()

public:
	
	ATimedSpotLight();
	
	virtual TArray<ULightComponent*> GetLightComponents() const override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spot")
	USpotLightComponent* SpotLight;

};