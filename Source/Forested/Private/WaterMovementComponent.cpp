#include "WaterMovementComponent.h"
#include "WaterActor.h"
#include "Kismet/KismetSystemLibrary.h"

UWaterMovementComponent::UWaterMovementComponent() {
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UWaterMovementComponent::BeginPlay() {
	Super::BeginPlay();

	OwningActor = GetOwner();
	check(OwningActor);
	
	OwnerRoot = Cast<UPrimitiveComponent>(OwningActor->GetRootComponent());
	if (!OwnerRoot) {
		LOG_ERROR("Physics Water Component could not find a primitive component as root in actor %s, please fix before continuing", *OwningActor->GetName());
	}
}

void UWaterMovementComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwningActor || !OwnerRoot) return;
	
	const AWaterActor* WaterActor = nullptr;
	float CurrentInputKey = 0.f;
	FVector WaterSplineLocation = FVector(TNumericLimits<double>::Lowest());

	const FCollisionQueryParams CollisionQueryParams(SCENE_QUERY_STAT(DefaultQueryParam), false);
	TArray<FHitResult> Hits;
	if (GetWorld()->SweepMultiByObjectType(Hits, OwningActor->GetActorLocation(), OwningActor->GetActorLocation(), FQuat::Identity, WATER_OBJECT_CHANNEL, FCollisionShape::MakeSphere(CollisionSphereRadius), CollisionQueryParams)) {
		for (const FHitResult& Result : Hits) {
			if (const AWaterActor* HitActor = Cast<AWaterActor>(Result.GetActor())) {
				const UWaterSplineComponent* WaterSpline = HitActor->SplineComponent;
				check(WaterSpline);

				const float InputKey = WaterSpline->FindInputKeyClosestToWorldLocation(OwningActor->GetActorLocation());
				const FVector HitActorWaterSplineLocation = WaterSpline->GetLocationAtSplineInputKey(InputKey, ESplineCoordinateSpace::World);
				if (HitActorWaterSplineLocation.Z > WaterSplineLocation.Z) {
					WaterActor = HitActor;
					CurrentInputKey = InputKey;
					WaterSplineLocation = HitActorWaterSplineLocation;
				}
			}
		}
	}
	
	bInWater = IsValid(WaterActor);
	if (!bInWater || !OwnerRoot->IsAnySimulatingPhysics()) return;

	const FVector Velocity = OwnerRoot->GetComponentVelocity();
	ApplyMovementForces(WaterActor, CurrentInputKey, DeltaTime, WaterSplineLocation);
	ApplyBuoyancy(WaterSplineLocation, Velocity);
	ApplyDrag(Velocity);
}

void UWaterMovementComponent::ApplyMovementForces(const AWaterActor* WaterActor, const float InputKey, const float DeltaTime, const FVector& WaterSplineLocation) const {
	if (!bApplyMovementForces) return;
	
	const float WaterSpeed = WaterActor->GetWaterSpeedAtInputKey(InputKey);
	if (WaterSpeed <= 0.f) return;
	
	const FVector ShoreDirection = (OwningActor->GetActorLocation() - WaterSplineLocation).GetSafeNormal2D();
	const float WaterPushAmount = WaterShorePushFactor * WaterActor->GetRiverAmountAtInputKey(InputKey);
	const FVector WaterDirection = WaterActor->GetWaterDirectionAtInputKey(InputKey) * (1 - WaterPushAmount) + ShoreDirection * WaterPushAmount;
	const FVector Acceleration = WaterDirection * WaterSpeed / DeltaTime * WaterVelocityStrength;
	const FVector WaterVelocity = Acceleration.GetClampedToSize(-MaxWaterForce, MaxWaterForce);
	OwnerRoot->AddForce(WaterVelocity, NAME_None, true);
}

void UWaterMovementComponent::ApplyBuoyancy(const FVector& WaterSplineLocation, const FVector& Velocity) const {
	if (!bApplyBuoyancy) return;
	const float Height = FMath::Min(CollisionSphereRadius * 2.f, WaterSplineLocation.Z - (OwningActor->GetActorLocation().Z - CollisionSphereRadius));
	const float Volume = PI / 3.f * (Height * Height) * (3.f * CollisionSphereRadius - Height);
	const float ForwardSpeed = FVector::DotProduct(OwnerRoot->GetForwardVector(), Velocity) * 0.036f;
	const float RampFactor = FMath::Clamp((ForwardSpeed - BuoyancyRampMinVelocity) / (BuoyancyRampMaxVelocity - BuoyancyRampMinVelocity), 0.f, 1.f);
	const float BuoyancyCoefficientWithRamp = BuoyancyCoefficient * (1 + RampFactor * (BuoyancyRampMax - 1));
	const float DampingFactor = -FMath::Max(0.f, BuoyancyDamp * Velocity.Z + FMath::Sign(Velocity.Z) * BuoyancyDamp2 * Velocity.Z * Velocity.Z);
	const float BuoyantForce = FMath::Clamp(Volume * BuoyancyCoefficientWithRamp + DampingFactor, 0.f, MaxBuoyantForce);

	OwnerRoot->AddForceAtLocation(FVector::UpVector * BuoyantForce, OwningActor->GetActorLocation(), NAME_None);
}

void UWaterMovementComponent::ApplyDrag(const FVector& Velocity) const {
	if (!bApplyDragForces) return;
	
	const float SpeedKmh = Velocity.Size2D() * 0.036f;
	const float ClampedSpeed = FMath::Clamp(SpeedKmh, -MaxDragSpeed, MaxDragSpeed);
	const float Resistance = DragCoefficient * ClampedSpeed + FMath::Sign(SpeedKmh) * DragCoefficient2 * ClampedSpeed * ClampedSpeed;
	OwnerRoot->AddForce(-Resistance * Velocity.GetSafeNormal2D(), NAME_None, true);
	OwnerRoot->AddTorqueInDegrees(-OwnerRoot->GetPhysicsAngularVelocityInDegrees() * AngularDragCoefficient, NAME_None, true);
}