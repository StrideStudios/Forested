#pragma once

#include "Forested/ForestedMinimal.h"
#include "Components/SplineRenderComponent.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interface_PostProcessVolume.h"
#include "WaterActor.generated.h"

class USplineComponent;
class USplineMeshComponent;

USTRUCT(BlueprintType)
struct FLODMaterial {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<UMaterialInterface*> Materials;
};

UENUM(BlueprintType)
enum class EWaterType : uint8 {
	Lake UMETA(DisplayName = "Lake"),
	River UMETA(DisplayName = "River"),
	LakeToRiver UMETA(DisplayName = "LakeToRiver"),
	RiverToLake UMETA(DisplayName = "RiverToLake")
};

USTRUCT(BlueprintType)
struct FWaterData {
	GENERATED_BODY()

	FWaterData() = default;
	
	FWaterData(const EWaterType WaterType, const float WaterSpeed = 0.f):
	WaterSpeed(WaterSpeed),
	WaterType(WaterType){
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WaterSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWaterType WaterType = EWaterType::Lake;
};

UCLASS(ClassGroup = Utility, meta=(BlueprintSpawnableComponent, PrioritizeCategories = "River Spline"))
class FORESTED_API UWaterSplineComponent : public USplineRenderComponent {
	GENERATED_BODY()

public:

	UWaterSplineComponent();

protected:

	virtual void BeginPlay() override;

public:

	virtual void RefreshSplineComponent() override;

	virtual void InitComponent(USplineMeshComponent* SplinePointMesh, int Index) override;

	virtual FPostProcessVolumeProperties GetPostProcessProperties() const {
		FPostProcessVolumeProperties Ret;
		Ret.bIsEnabled = bEnabled;
		Ret.bIsUnbound = false;
		Ret.BlendRadius = BlendRadius;
		Ret.BlendWeight = BlendWeight;
		Ret.Priority = Priority;
		Ret.Settings = &Settings;
		return Ret;
	}

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "River")
	TMap<EWaterType, FLODMaterial> MaterialMap;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "River")
	TArray<FWaterData> WaterData;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "River|Post Process")
	bool bEnabled = true;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "River|Post Process")
	float BlendRadius = 100.f;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "River|Post Process")
	float BlendWeight = 1.f;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "River|Post Process")
	int Priority = 1;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "River|Post Process")
	FPostProcessSettings Settings;
};

UCLASS(meta=(PrioritizeCategories = "Material Mesh"))
class FORESTED_API AWaterActor : public AActor {
	GENERATED_BODY()
	
public:	
	AWaterActor();

protected:

	virtual void BeginPlay() override;

public:

	virtual void Tick(float DeltaSeconds) override;

	virtual void OnConstruction(const FTransform& Transform) override;

	float GetRiverAmountAtInputKey(float InputKey) const;
	
	FVector GetWaterDirectionAtInputKey(float InputKey) const;
	
	float GetWaterSpeedAtInputKey(float InputKey) const;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UWaterSplineComponent* SplineComponent;
	
};
