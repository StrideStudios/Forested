#pragma once

#include "Forested/ForestedMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "ViewmodelMeshes.generated.h"

class AFPlayer;

USTRUCT(BlueprintType)
struct FViewmodelData {
	GENERATED_BODY()

	//whether to use the viewmodel fov or not
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (InlineEditConditionToggle), Category = "Viewmodel")
	bool bUseViewmodelFOV = true;

	//the fov that will apply to the viewmodel
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition="bUseViewmodelFOV", UIMin = "5.0", UIMax = "170", ClampMin = "0.001", ClampMax = "360.0", Units = deg), Category = "Viewmodel")
	float ViewmodelFOV = 90.0f;

	//whether to use the viewmodel scale or not
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (InlineEditConditionToggle), Category = "Viewmodel")
	bool bUseViewmodelScale = true;
	
	//scales the mesh toward or away from the camera
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition="bUseViewmodelScale", UIMin = "0.0", UIMax = "1.0"), Category = "Viewmodel")
	float ViewmodelScale = 1.0f;
};

UCLASS()
class FORESTED_API UViewmodelMeshes : public UBlueprintFunctionLibrary {
	GENERATED_BODY()
	
public:

	//correct a position attached to camera to fit with a certain viewmodel fov or scale
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = 1), Category = "Viewmodel")
	static FVector CalculateViewmodelLocation(const APlayerController* PlayerController, const FVector& Location, const FViewmodelData& ViewmodelData);

	//correct a rotation attached to camera to fit with a certain viewmodel fov or scale
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = 1), Category = "Viewmodel")
	static FRotator CalculateViewmodelRotation(const APlayerController* PlayerController, const FRotator& Rotation, const FViewmodelData& ViewmodelData);
	
	//correct a transform attached to camera to fit with a certain viewmodel fov or scale
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = 1), Category = "Viewmodel")
	static FTransform CalculateViewmodelTransform(const APlayerController* PlayerController, const FTransform& Transform, const FViewmodelData& ViewmodelData);
	
	static void CalculateViewmodelMatrix(const APlayerController* PlayerController, const FViewmodelData& ViewmodelData, FMatrix& InOutMatrix);

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

	//the viewmodel mesh data
	UPROPERTY(EditAnywhere, Category = "Viewmodel")
	FViewmodelData ViewmodelData;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Viewmodel")
	FORCEINLINE FViewmodelData GetViewmodelData() const { return ViewmodelData; }
	
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

	//the viewmodel mesh data
	UPROPERTY(EditAnywhere, Category = "Viewmodel")
	FViewmodelData ViewmodelData;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Viewmodel")
	FORCEINLINE FViewmodelData GetViewmodelData() const { return ViewmodelData; }
	
protected:

	virtual void BeginPlay() override;

	virtual FMatrix GetRenderMatrix() const override;

private:
	UPROPERTY()
	APlayerController* PlayerController = nullptr;
	
};
