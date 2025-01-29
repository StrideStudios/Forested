#pragma once

#include "CoreMinimal.h"
#include "PlayerInventoryActor.h"
#include "ThrowableInventoryRenderActor.generated.h"

struct FPredictProjectilePathResult;
class USphereComponent;
class USplineRenderComponent;
class UNiagaraComponent;
class UCameraShakeBase;
class UNiagaraSystem;
class UCurveVector;

UCLASS(BlueprintType)
class FORESTED_API AThrowableTargetActor : public AActor {
	GENERATED_BODY()

public:

	AThrowableTargetActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* SphereComponent;
};

UCLASS(BlueprintType, meta = (PrioritizeCategories = "Throwable Components"))
class FORESTED_API AThrowableInventoryRenderActor : public APlayerInventoryActor {
	GENERATED_BODY()

public:
	
	AThrowableInventoryRenderActor();

	virtual void Init() override;
	
	virtual void InventoryTick(const float DeltaTime) override;

	virtual void OnLeftInteract() override;

	virtual void OnRightInteract() override;

	virtual void OnRightEndInteract() override;

	virtual void OnMontageNotifyBegin(const UAnimMontage* Montage, const FName Notify) override;

	UFUNCTION(BlueprintCallable, Category = "Throwable Inventory Render Actor")
	void ThrowItem() const;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USplineRenderComponent* Spline;
	
protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable")
	UAnimMontage* ThrowAnimMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable")
	float ProjectileRadius = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable")
	float ImpulseVelocity = 1000.f;

	UPROPERTY(BlueprintReadWrite, Category = "Throwable")
	bool HoldingRightClick = false;

private:
	
	void GetLaunchVelocity(FVector& OutVelocity) const;

	bool GetPathResult(const FVector& StartLocation, const FVector& LaunchVelocity, FPredictProjectilePathResult& Result) const;
	
	bool GetLaunchVelocity_Internal(const FVector& StartLocation, const FVector& EndLocation, const TArray<AActor*>& IgnoredActors, FVector& OutVelocity) const;

};
