#include "Items/ThrowableInventoryRenderActor.h"
#include "Player/FPlayer.h"
#include "Components/SplineRenderComponent.h"
#include "Player/ViewmodelMeshes.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Items/ItemActor.h"
#include "Player/PlayerInventory.h"
#include "Kismet/GameplayStatics.h"

AThrowableTargetActor::AThrowableTargetActor() {
	PrimaryActorTick.bCanEverTick = false;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Component"));
	RootComponent = SphereComponent;
	SphereComponent->SetCollisionResponseToChannel(HOVER_TRACE_CHANNEL, ECR_Overlap);
	SphereComponent->SetCollisionResponseToChannel(SELECT_TRACE_CHANNEL, ECR_Ignore);
}

AThrowableInventoryRenderActor::AThrowableInventoryRenderActor() {
	PrimaryActorTick.bCanEverTick = false;

	Spline = CreateDefaultSubobject<USplineRenderComponent>(TEXT("Spline"));
	Spline->SetupAttachment(GetRootComponent());
}

void AThrowableInventoryRenderActor::Init() {
	Super::Init();
	Spline->ClearSplinePoints();
	Spline->ClearComponents();
}

void AThrowableInventoryRenderActor::InventoryTick(const float DeltaTime) {
	Super::InventoryTick(DeltaTime);
	if (HoldingRightClick) {
		FVector LaunchVelocity;
		GetLaunchVelocity(LaunchVelocity);
		if (!Spline || LaunchVelocity.IsNearlyZero()) return;
		FPredictProjectilePathResult Result;
		if (!GetPathResult(Spline->GetComponentLocation(), LaunchVelocity, Result)) return;
		Spline->ClearSplinePoints();
		Spline->ClearComponents();
		for (FPredictProjectilePathPointData Point : Result.PathData) {
			Spline->AddSplinePoint(Point.Location, ESplineCoordinateSpace::World, false);
		}
		Spline->UpdateSpline();
		Spline->RefreshSplineComponent();
	}
}

void AThrowableInventoryRenderActor::OnLeftInteract() {
	Super::OnLeftInteract();
	if (!HoldingRightClick || GetPlayerAnimInstance()->GetCurrentActiveMontage() != ThrowAnimMontage) return;
	if (StopMontage(0.1f, ThrowAnimMontage))
		ThrowItem();
}

void AThrowableInventoryRenderActor::OnRightInteract() {
	Super::OnRightInteract();
	StartMontage(ThrowAnimMontage);
}

void AThrowableInventoryRenderActor::OnRightEndInteract() {
	Super::OnRightEndInteract();
	HoldingRightClick = false;
	Spline->ClearSplinePoints();
	Spline->ClearComponents();
	if (GetPlayerAnimInstance()->GetCurrentActiveMontage() != ThrowAnimMontage) return;
	if (!ResumeMontage(ThrowAnimMontage)) {
		StopMontage(0.25f, ThrowAnimMontage);
	}
}

void AThrowableInventoryRenderActor::OnMontageNotifyBegin(const UAnimMontage* Montage, const FName Notify) {
	Super::OnMontageNotifyBegin(Montage, Notify);
	if (GetPlayerAnimInstance()->GetCurrentActiveMontage() == ThrowAnimMontage && Notify == "PauseMontage") {
		PauseMontage(Montage);
		HoldingRightClick = true;
	}
}

void AThrowableInventoryRenderActor::ThrowItem() const {
	FItemHeap ItemHeap;
	if (!GetItem(ItemHeap)) return;
	FVector LaunchVelocity;
	GetLaunchVelocity(LaunchVelocity);
	if (LaunchVelocity.IsNearlyZero()) return;
	//TODO: adjust velocity to match spline location
	AItemActor::SpawnItemActor(GetWorld(), StaticMeshComponent->GetComponentTransform(), ItemHeap.Top(), [](const AItemActor* Item) {
		PLAYER_INVENTORY->RemoveItem(PLAYER_INVENTORY->GetSelectedSlot());
	}, LaunchVelocity);
}

void AThrowableInventoryRenderActor::GetLaunchVelocity(FVector& OutVelocity) const {
	OutVelocity = PLAYER->Camera->GetForwardVector() * ImpulseVelocity;
	if (AThrowableTargetActor* TargetActor = Cast<AThrowableTargetActor>(PLAYER->GetHoveredHitResult().GetActor())) {
		GetLaunchVelocity_Internal(Spline->GetComponentLocation(), TargetActor->GetActorLocation(), { PLAYER, TargetActor}, OutVelocity);
	}
}

bool AThrowableInventoryRenderActor::GetPathResult(const FVector& StartLocation, const FVector& LaunchVelocity, FPredictProjectilePathResult& Result) const {
	const FPredictProjectilePathParams& Params = FPredictProjectilePathParams(ProjectileRadius, StartLocation, LaunchVelocity, 5.f, ECC_Visibility);
	return UGameplayStatics::PredictProjectilePath(GetWorld(), Params, Result);
}

bool AThrowableInventoryRenderActor::GetLaunchVelocity_Internal(const FVector& StartLocation, const FVector& EndLocation, const TArray<AActor*>& IgnoredActors, FVector& OutVelocity) const {
	//always does a world-dynamic trace
	return UGameplayStatics::SuggestProjectileVelocity(GetWorld(), OutVelocity, StartLocation, EndLocation, ImpulseVelocity,
		false, ProjectileRadius, 0, ESuggestProjVelocityTraceOption::TraceFullPath, FCollisionResponseParams::DefaultResponseParam, IgnoredActors);
}