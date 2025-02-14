#pragma once

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
	
	virtual void OnConstruction(const FTransform& Transform) override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USceneComponent* Root;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Environment Light")
	void RefreshLight(float DeltaTime, const ASky* Sky);
	virtual void RefreshLight_Implementation(float DeltaTime, const ASky* Sky);

	virtual TArray<ULightComponent*> GetLightComponents() const {
		return {};
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base")
	UCurveFloat* IntensityCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
	float DayIntensity = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
	float DayTemperature = 5500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
	float NightIntensity = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
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

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void RefreshLight_Implementation(float DeltaTime, const ASky* Sky) override;
	
	virtual TArray<ULightComponent*> GetLightComponents() const override;
	
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