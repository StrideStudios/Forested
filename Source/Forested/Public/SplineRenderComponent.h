#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "SplineRenderComponent.generated.h"

class USplineMeshComponent;

UCLASS(ClassGroup = Utility, meta=(BlueprintSpawnableComponent, PrioritizeCategories = "Spline"))
class FORESTED_API USplineRenderComponent : public USplineComponent {
	GENERATED_BODY()

	friend class ASplineRenderActor;

public:

	USplineRenderComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Spline Render Component")
	virtual void RefreshSplineComponent();

	UFUNCTION(BlueprintCallable, Category = "Spline Render Component")
	virtual void UpdateSplineComponent(USplineMeshComponent* SplinePointMesh, int32 Index) const;

	UFUNCTION(BlueprintCallable, Category = "Spline Render Component")
	void ClearComponents();
	
	virtual void OnRegister() override;
	
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	virtual void DestroyComponent(bool bPromoteChildren = false) override;
	
	virtual TSubclassOf<USplineMeshComponent> GetSplineClass() const {
		return USplineMeshComponent::StaticClass();
	}

	UFUNCTION(BlueprintCallable, Category = "Spline Render Component")
	int GetNumberOfSplineComponents() const { return IsClosedLoop() ? GetNumberOfSplinePoints() : GetNumberOfSplinePoints() - 1; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spline")
	UStaticMesh* Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spline")
	FVector2D MeshScale = FVector2D(1.0);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spline")
	TArray<UMaterialInterface*> Materials;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spline")
	TEnumAsByte<ESplineMeshAxis::Type> SplineForwardAxis = ESplineMeshAxis::X;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spline")
	bool AllowSplineRoll = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spline")
	bool OverlapEvents = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spline")
	TEnumAsByte<ECollisionEnabled::Type> CollisionType = ECollisionEnabled::QueryAndPhysics;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spline")
	TEnumAsByte<ECollisionChannel> CollisionObjectType = ECC_WorldStatic;

protected:

	virtual void InitComponent(USplineMeshComponent* SplinePointMesh, int Index);

private:

	UPROPERTY()
	TArray<USplineMeshComponent*> SplineComponents;
	
};

UCLASS()
class FORESTED_API ASplineRenderActor : public AActor {
	GENERATED_BODY()

public:

	ASplineRenderActor();

	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USplineRenderComponent* SplineComponent;
};
