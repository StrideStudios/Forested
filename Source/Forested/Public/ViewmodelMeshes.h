#pragma once

#include "Forested/ForestedMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "ViewmodelMeshes.generated.h"

class AFPlayer;

class FViewmodelMeshes {
	
public:
	static void CalculateViewmodelMatrix(const APlayerController* PlayerController, bool bUseViewmodelFOV, float ViewmodelFOV, bool bUseViewmodelScale, float ViewmodelScale, FMatrix& InOutMatrix);

	static double CorrectFOVRad(const float TargetHorizontalFOV, const double TargetAspect, const double CurrentAspect) {
		const double TargetHalfFOVRad = TargetHorizontalFOV * PI / 360.0;
		const double DesiredHalfFOVRad = FMath::Atan(FMath::Tan(TargetHalfFOVRad) / TargetAspect);
		return FMath::Tan(DesiredHalfFOVRad) * CurrentAspect;
	}

};

UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class FORESTED_API UViewmodelStaticMeshComponent : public UStaticMeshComponent {
	GENERATED_BODY()
	
public:

	UViewmodelStaticMeshComponent();

	//whether to use the viewmodel fov or not
	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle), Category = "Viewmodel")
	bool bUseViewmodelFOV = true;

	//the fov that will apply to the viewmodel
	UPROPERTY(EditAnywhere, meta = (EditCondition="bUseViewmodelFOV", UIMin = "5.0", UIMax = "170", ClampMin = "0.001", ClampMax = "360.0", Units = deg), Category = "Viewmodel")
	float ViewmodelFOV = 90.0f;

	//whether to use the viewmodel scale or not
	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle), Category = "Viewmodel")
	bool bUseViewmodelScale = true;
	
	//scales the mesh toward or away from the camera
	UPROPERTY(EditAnywhere, meta = (EditCondition="bUseViewmodelScale", UIMin = "0.0", UIMax = "1.0"), Category = "Viewmodel")
	float ViewmodelScale = 1.0f;

protected:

	virtual void BeginPlay() override;

	virtual FMatrix GetRenderMatrix() const override;

private:
	UPROPERTY()
	APlayerController* PlayerController = nullptr;
	
};

UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class FORESTED_API UViewmodelSkeletalMeshComponent : public USkeletalMeshComponent {
	GENERATED_BODY()
	
public:

	UViewmodelSkeletalMeshComponent();

	//whether to use the viewmodel fov or not
	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle), Category = "Viewmodel")
	bool bUseViewmodelFOV = true;

	//the fov that will apply to the viewmodel
	UPROPERTY(EditAnywhere, meta = (EditCondition="bUseViewmodelFOV", UIMin = "5.0", UIMax = "170", ClampMin = "0.001", ClampMax = "360.0", Units = deg), Category = "Viewmodel")
	float ViewmodelFOV = 90.0f;

	//whether to use the viewmodel scale or not
	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle), Category = "Viewmodel")
	bool bUseViewmodelScale = true;
	
	//scales the mesh toward or away from the camera
	UPROPERTY(EditAnywhere, meta = (EditCondition="bUseViewmodelScale", UIMin = "0.0", UIMax = "1.0"), Category = "Viewmodel")
	float ViewmodelScale = 1.0f;

protected:

	virtual void BeginPlay() override;

	virtual FMatrix GetRenderMatrix() const override;

private:
	UPROPERTY()
	APlayerController* PlayerController = nullptr;
	
};
