#pragma once

#include "Forested/ForestedMinimal.h"
#include "Components/ActorComponent.h"
#include "DifferentialIKComponent.generated.h"


UCLASS(ClassGroup = Utility, Blueprintable, meta = (BlueprintSpawnableComponent, PrioritizeCategories = "Dynamics"))
class FORESTED_API UDifferentialIKComponent : public USceneComponent {
	GENERATED_BODY()

public:	
	UDifferentialIKComponent() {
		PrimaryComponentTick.bCanEverTick = true;
	}

protected:
	virtual void BeginPlay() override {
		Super::BeginPlay();
		ResetDynamics();
	}

	static FORCEINLINE FQuat ShortestPath(const FQuat& A, const FQuat& B) {
		const float DotProduct =
			A.X * B.X +
			A.Y * B.Y +
			A.Z * B.Z +
			A.W * B.W;
		return A * FMath::FloatSelect(DotProduct, 1.f, -1.f);
	}

	FTransform TargetTransform;
	FVector PTargetLocation, CurrentLocation, DeltaLocation;
	FQuat PTargetRotation, CurrentRotation, DeltaRotation;

	float K1 = 0, K2 = 0, K3 = 0 ;

public:	
	UFUNCTION(BlueprintCallable)
	void TickDifferentialIK(const float DeltaTime) {
		const FVector DeltaTargetLocation = (TargetTransform.GetLocation() - PTargetLocation) / DeltaTime;
		const FQuat DeltaTargetRotation = (ShortestPath(TargetTransform.GetRotation(), CurrentRotation) - PTargetRotation) / DeltaTime;
		PTargetLocation = TargetTransform.GetLocation();
		PTargetRotation = ShortestPath(TargetTransform.GetRotation(), CurrentRotation);
		CurrentLocation = CurrentLocation + DeltaTime * DeltaLocation;
		CurrentRotation = CurrentRotation + DeltaRotation * DeltaTime;
		const float SK2 = FMath::Max(K2, 1.1f * (DeltaTime * DeltaTime / 4.f + DeltaTime * K1 / 2));
		DeltaLocation = DeltaLocation + (TargetTransform.GetLocation() + DeltaTargetLocation * K3 - CurrentLocation - DeltaLocation * K1) * DeltaTime / SK2;
		DeltaRotation = DeltaRotation + (ShortestPath(TargetTransform.GetRotation(), CurrentRotation) + DeltaTargetRotation * K3 - CurrentRotation - DeltaRotation * K1) * DeltaTime / SK2;
		CurrentRotation.Normalize();
		SetRelativeTransform(FTransform(CurrentRotation, CurrentLocation, TargetTransform.GetScale3D()), bShouldSweep);
	}
	
	UFUNCTION(BlueprintCallable)
	void KillVelocity() {
		DeltaLocation = FVector(0.f);
		DeltaRotation = FQuat(0.f,0.f,0.f,1.f);
	}

	UFUNCTION(BlueprintCallable)
	void ForceTransform(const FTransform Transform) {
		ForceLocation(Transform.GetLocation());
		ForceRotation(Transform.GetRotation());
	}
	
	UFUNCTION(BlueprintCallable)
	void ForceLocation(const FVector Location) {
		SetRelativeLocation(Location);
		CurrentLocation = Location;
	}

	void ForceRotation(const FQuat& Rotation) {
		SetRelativeRotation(Rotation);
		CurrentRotation = Rotation;
	}
	
	UFUNCTION(BlueprintCallable)
	void ForceRotation(const FRotator Rotation) {
		ForceRotation(FQuat(Rotation));
	}

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE bool IsAtTarget(const float Tolerance) const {
		return GetRelativeTransform().Equals(TargetTransform, Tolerance);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE FTransform GetTargetTransform() const {
		return TargetTransform;
	}

	UFUNCTION(BlueprintCallable)
	void SetTargetTransform(const FTransform Transform) {
		TargetTransform = Transform;
	}

	UFUNCTION(BlueprintCallable)
	void SetSpeed(const float NewSpeed) {
		Speed = NewSpeed;
		ResetDynamics();
	}

	UFUNCTION(BlueprintCallable)
	void SetSettle(const float NewSettle) {
		Settle = NewSettle;
		ResetDynamics();
	}
	
	UFUNCTION(BlueprintCallable)
	void SetResponse(const float NewResponse) {
		Response = NewResponse;
		ResetDynamics();
	}

	UFUNCTION(BlueprintCallable)
	void ResetDynamics() {
		K1 = Settle / (Speed * PI);
		K2 = 1 / (2 * Speed * PI * (2 * Speed * PI));
		K3 = Response * Settle / (2 * Speed * PI);
	}
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Dynamics")
	bool bShouldSweep = false;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Dynamics")
	float Speed = 2.5f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Dynamics")
	float Settle = 0.5f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Dynamics")
	float Response = 1.f;
};
