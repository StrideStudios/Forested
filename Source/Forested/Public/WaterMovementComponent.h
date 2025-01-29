#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WaterMovementComponent.generated.h"

class AWaterActor;

UCLASS(ClassGroup = Utility, meta=(BlueprintSpawnableComponent, PrioritizeCategories = "Water"))
class FORESTED_API UWaterMovementComponent : public UActorComponent {
	GENERATED_BODY()

public:

	UWaterMovementComponent();

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ApplyMovementForces(const AWaterActor* WaterActor, float InputKey, float DeltaTime, const FVector& WaterSplineLocation) const;

	void ApplyBuoyancy(const FVector& WaterSplineLocation, const FVector& Velocity) const;
	
	void ApplyDrag(const FVector& Velocity) const;

	
	FORCEINLINE bool IsInWater() const { return bInWater; }

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Water")
	float CollisionSphereRadius = 100.f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Water|Buoyancy")
	bool bApplyBuoyancy = true;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Water|Buoyancy")
	float BuoyancyRampMinVelocity = 20.f;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Water|Buoyancy")
	float BuoyancyRampMaxVelocity = 50.f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Water|Buoyancy")
	float BuoyancyRampMax = 1.f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Water|Buoyancy")
	float BuoyancyCoefficient = 0.1f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Water|Buoyancy")
	float BuoyancyDamp = 1000.f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Water|Buoyancy")
	float BuoyancyDamp2 = 1.f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Water|Buoyancy")
	float MaxBuoyantForce = 5000000.f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Water|Drag")
	bool bApplyDragForces = true;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Water|Drag")
	float MaxDragSpeed = 15.f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Water|Drag")
	float DragCoefficient = 20.f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Water|Drag")
	float DragCoefficient2 = 0.01f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Water|Drag")
	float AngularDragCoefficient = 1.f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Water|Movement")
	bool bApplyMovementForces = true;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Water|Movement")
	float WaterVelocityStrength = 0.01f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Water|Movement")
	float MaxWaterForce = 10000.f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Water|Movement|River")
	float WaterShorePushFactor = 0.3f;
	
private:

	UPROPERTY()
	UPrimitiveComponent* OwnerRoot = nullptr;

	UPROPERTY()
	AActor* OwningActor = nullptr;

	bool bInWater = false;
	
};