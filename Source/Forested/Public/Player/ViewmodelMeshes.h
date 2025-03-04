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

USTRUCT(BlueprintType)
struct FViewmodelVector {
	GENERATED_BODY()

	//the viewmodel data to use
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewmodel")
	FViewmodelData ViewmodelData;

	//the vector to apply the viewmodel data to
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewmodel")
	FVector Vector;

	operator FVector() const;
	
};

USTRUCT(BlueprintType)
struct FViewmodelRotator {
	GENERATED_BODY()

	//the viewmodel data to use
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewmodel")
	FViewmodelData ViewmodelData;

	//the rotator to apply the viewmodel data to
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewmodel")
	FRotator Rotation;

	operator FRotator() const;
	
};

USTRUCT(BlueprintType)
struct FViewmodelTransform {
	GENERATED_BODY()

	//the viewmodel data to use
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewmodel")
	FViewmodelData ViewmodelData;

	//the transform to apply the viewmodel data to
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewmodel")
	FTransform Transform;

	operator FTransform() const;
	
};

UCLASS()
class FORESTED_API UViewmodelMeshes : public UBlueprintFunctionLibrary {
	GENERATED_BODY()
	
public:
	
	//correct a vector attached to camera to fit with a certain viewmodel fov or scale
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Convert To Vector", CompactNodeTitle = "->", BlueprintAutocast), Category = "Viewmodel")
	static FVector Conv_ViewmodelVectorToVector(const FViewmodelVector& Vector);
	
	//correct a rotator attached to camera to fit with a certain viewmodel fov or scale
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Convert To Rotator", CompactNodeTitle = "->", BlueprintAutocast), Category = "Viewmodel")
	static FRotator Conv_ViewmodelRotatorToRotator(const FViewmodelRotator& Rotator);
	
	//correct a transform attached to camera to fit with a certain viewmodel fov or scale
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Convert To Transform", CompactNodeTitle = "->", BlueprintAutocast), Category = "Viewmodel")
	static FTransform Conv_ViewmodelTransformToTransform(const FViewmodelTransform& Transform);
	
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
