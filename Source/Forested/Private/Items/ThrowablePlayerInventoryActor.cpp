#include "Items/ThrowablePlayerInventoryActor.h"

#include "MontageLibrary.h"
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

AThrowablePlayerInventoryActor::AThrowablePlayerInventoryActor() {
	PrimaryActorTick.bCanEverTick = false;

	Spline = CreateDefaultSubobject<USplineRenderComponent>(TEXT("Spline"));
	Spline->SetupAttachment(GetRootComponent());
}

void AThrowablePlayerInventoryActor::Init(AFPlayer* InPlayer) {
	Super::Init(InPlayer);
	Spline->ClearSplinePoints();
	Spline->ClearComponents();
}

void AThrowablePlayerInventoryActor::InventoryTick(const float DeltaTime) {
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

void AThrowablePlayerInventoryActor::OnLeftInteract() {
	Super::OnLeftInteract();
	if (!HoldingRightClick || GetPlayerAnimInstance()->GetCurrentActiveMontage() != ThrowAnimMontage) return;
	if (UMontageLibrary::StopMontage(GetPlayer()->GetMesh(), ThrowAnimMontage, 0.1f))
		ThrowItem();
}

void AThrowablePlayerInventoryActor::OnRightInteract() {
	Super::OnRightInteract();
	UMontageLibrary::StartMontage(GetPlayer()->GetMesh(), ThrowAnimMontage);
}

void AThrowablePlayerInventoryActor::OnRightEndInteract() {
	Super::OnRightEndInteract();
	HoldingRightClick = false;
	Spline->ClearSplinePoints();
	Spline->ClearComponents();
	if (GetPlayerAnimInstance()->GetCurrentActiveMontage() != ThrowAnimMontage) return;
	if (!UMontageLibrary::ResumeMontage(GetPlayer()->GetMesh(), ThrowAnimMontage)) {
		UMontageLibrary::StopMontage(GetPlayer()->GetMesh(), ThrowAnimMontage, 0.25f);
	}
}

void AThrowablePlayerInventoryActor::OnMontageNotifyBegin(const UAnimMontage* Montage, const FName Notify) {
	Super::OnMontageNotifyBegin(Montage, Notify);
	if (GetPlayerAnimInstance()->GetCurrentActiveMontage() == ThrowAnimMontage && Notify == "PauseMontage") {
		UMontageLibrary::PauseMontage(GetPlayer()->GetMesh(), Montage);
		HoldingRightClick = true;
	}
}

void AThrowablePlayerInventoryActor::ThrowItem() const {
	FItemHeap ItemHeap;
	if (!GetItem(ItemHeap)) return;
	FVector LaunchVelocity;
	GetLaunchVelocity(LaunchVelocity);
	if (LaunchVelocity.IsNearlyZero()) return;
	//TODO: adjust velocity to match spline location
	AItemActor::SpawnItemActor(GetWorld(), StaticMeshComponent->GetComponentTransform(), ItemHeap.Top(), LaunchVelocity, FVector(0.0), [this](const AItemActor* Item) {
		GetPlayer()->PlayerInventory->RemoveItem(GetPlayer()->PlayerInventory->GetSelectedSlot());
	});
}

void AThrowablePlayerInventoryActor::GetLaunchVelocity(FVector& OutVelocity) const {
	OutVelocity = GetPlayer()->Camera->GetForwardVector() * ImpulseVelocity;
	if (AThrowableTargetActor* TargetActor = Cast<AThrowableTargetActor>(GetPlayer()->GetHoveredHitResult().GetActor())) {
		GetLaunchVelocity_Internal(Spline->GetComponentLocation(), TargetActor->GetActorLocation(), { GetPlayer(), TargetActor}, OutVelocity);
	}
}

bool AThrowablePlayerInventoryActor::GetPathResult(const FVector& StartLocation, const FVector& LaunchVelocity, FPredictProjectilePathResult& Result) const {
	const FPredictProjectilePathParams& Params = FPredictProjectilePathParams(ProjectileRadius, StartLocation, LaunchVelocity, 5.f, ECC_Visibility);
	return UGameplayStatics::PredictProjectilePath(GetWorld(), Params, Result);
}

bool AThrowablePlayerInventoryActor::GetLaunchVelocity_Internal(const FVector& StartLocation, const FVector& EndLocation, const TArray<AActor*>& IgnoredActors, FVector& OutVelocity) const {
	//always does a world-dynamic trace
	return UGameplayStatics::SuggestProjectileVelocity(GetWorld(), OutVelocity, StartLocation, EndLocation, ImpulseVelocity,
		false, ProjectileRadius, 0, ESuggestProjVelocityTraceOption::TraceFullPath, FCollisionResponseParams::DefaultResponseParam, IgnoredActors);
}