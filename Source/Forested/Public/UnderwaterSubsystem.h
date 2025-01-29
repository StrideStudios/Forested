#pragma once

#include "Interfaces/Interface_PostProcessVolume.h"
#include "Subsystems/WorldSubsystem.h"
#include "UnderwaterSubsystem.generated.h"

class AWaterActor;

struct FUnderwaterPostProcessVolume final : IInterface_PostProcessVolume {
	FUnderwaterPostProcessVolume():
	PostProcessProperties() {
	}

	virtual bool EncompassesPoint(FVector Point, float SphereRadius, float* OutDistanceToPoint) override {
		// For underwater, the distance to point is 0 for now because underwater doesn't look correct if it is blended with other post process due to the wave masking
		if (OutDistanceToPoint) {
			*OutDistanceToPoint = 0;
		}

		return PostProcessProperties.bIsEnabled && PostProcessProperties.Settings;
	}

	virtual FPostProcessVolumeProperties GetProperties() const override {
		return PostProcessProperties;
	}

	FPostProcessVolumeProperties PostProcessProperties;
};

UCLASS(BlueprintType, Transient)
class FORESTED_API UUnderwaterSubsystem : public UWorldSubsystem {
	GENERATED_BODY()

public:
	//UWorldSubsystem implementation
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

	//USubsystem Implementation
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	FORCEINLINE UMaterialParameterCollection* GetMaterialParameterCollection() const {	return MaterialParameterCollection; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Underwater Subsystem")
	FORCEINLINE float GetCameraDepth() const { return CameraDepth; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Underwater Subsystem")
	FORCEINLINE float GetLeftZHeight() const { return LeftZHeight; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Underwater Subsystem")
	FORCEINLINE float GeRightZHeight() const { return RightZHeight; }

private:
	
	void ComputeUnderwaterPostProcess(FVector ViewLocation, FSceneView* SceneView);

	void UpdateSceneProperties(UWorld* World, FSceneView* SceneView);
	
	void SetNotInWater();
	
	UPROPERTY()
	UMaterialParameterCollection* MaterialParameterCollection;

	UPROPERTY()
	AWaterActor* WaterActor;

	FUnderwaterPostProcessVolume UnderwaterPostProcessVolume;
	
	bool bIsInWater;
	
	double CameraDepth;
	
	double LeftZHeight;
	
	double RightZHeight;

};