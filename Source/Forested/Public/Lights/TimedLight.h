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
	
	virtual void OnConstruction(const FTransform& Transform) override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USceneComponent* Root;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Timed Light")
	void RefreshLight(float DeltaTime, const ASky* Sky);
	virtual void RefreshLight_Implementation(float DeltaTime, const ASky* Sky);

	virtual TArray<ULightComponent*> GetLightComponents() const {
		return {};
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base")
	UCurveFloat* IntensityCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
	float MinTime = -1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
	float MaxTime = 0.f;

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