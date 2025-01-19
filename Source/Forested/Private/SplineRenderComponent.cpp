#include "SplineRenderComponent.h"
#include "LevelDefaults.h"
#include "Components/SplineMeshComponent.h"

USplineRenderComponent::USplineRenderComponent() {
	PrimaryComponentTick.bCanEverTick = false;
}

void USplineRenderComponent::BeginPlay() {
	Super::BeginPlay();
	RefreshSplineComponent();
}

void USplineRenderComponent::RefreshSplineComponent() {
	//ClearComponents();

	AActor* Outer = GetOwner();
	if (!Outer || !IN_WORLD) return;
	
	while (SplineComponents.Num() != GetNumberOfSplineComponents()) {
		if (SplineComponents.Num() > GetNumberOfSplineComponents()) {
			const int i = SplineComponents.Num() - 1;
			USplineMeshComponent* SplinePointMesh = SplineComponents[i];//array out of bounds -1 into an array of size 0 (?)
			SplineComponents.RemoveAt(i);
			SplinePointMesh->DestroyComponent();
		} else if (SplineComponents.Num() < GetNumberOfSplineComponents()) {
			const int i = SplineComponents.Num();
			FString Name = FString::Printf(TEXT("SplineMeshComponent_%d"), i);
			USplineMeshComponent* SplinePointMesh = NewObject<USplineMeshComponent>(Outer, GetSplineClass(), *Name, RF_Transactional);
			SplinePointMesh->bSelectable = false;
			check(SplinePointMesh);
			SplineComponents.Add(SplinePointMesh);
		}
	}

	for (int32 i = 0; i < SplineComponents.Num(); ++i) {
		USplineMeshComponent* SplinePointMesh = SplineComponents[i];
		if (!SplinePointMesh) continue;
		InitComponent(SplinePointMesh, i);
		UpdateSplineComponent(SplinePointMesh, i);
	}
}

void USplineRenderComponent::UpdateSplineComponent(USplineMeshComponent* SplinePointMesh, const int32 Index) const {
	const int32 NextIndex = IsClosedLoop() && Index == GetNumberOfSplinePoints() - 1 ? 0 : Index + 1;

	const FVector StartScale = GetScaleAtSplinePoint(Index);
	const FVector EndScale = GetScaleAtSplinePoint(NextIndex);
	
	SplinePointMesh->SetStartScale(FVector2D(StartScale.Y, StartScale.Z) * MeshScale, false);
	SplinePointMesh->SetEndScale(FVector2D(EndScale.Y, EndScale.Z) * MeshScale, false);

	const FVector StartPos = GetLocationAtSplinePoint(Index, ESplineCoordinateSpace::Local);
	const FVector StartTangent = GetTangentAtSplinePoint(Index, ESplineCoordinateSpace::Local);

	//is slightly above at end?
	const FVector EndPos = GetLocationAtSplinePoint(NextIndex, ESplineCoordinateSpace::Local);
	const FVector EndTangent = GetTangentAtSplinePoint(NextIndex, ESplineCoordinateSpace::Local);

	SplinePointMesh->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent, false);

	SplinePointMesh->SetStartRoll(0.f, false);
	if (AllowSplineRoll) {
		const float StartRoll = FMath::DegreesToRadians(GetRollAtSplinePoint(Index, ESplineCoordinateSpace::Local));
		const float EndRoll = FMath::DegreesToRadians(GetRollAtSplinePoint(NextIndex, ESplineCoordinateSpace::Local));
		SplinePointMesh->SetStartRoll(StartRoll, false);
		SplinePointMesh->SetEndRoll(EndRoll, false);
	}
	SplinePointMesh->bMeshDirty = true;
	SplinePointMesh->UpdateMesh();
}

void USplineRenderComponent::ClearComponents() {
	for (const auto& Component : SplineComponents) {
		if (!IsValid(Component)) continue;
		Component->DestroyComponent();
	}
	SplineComponents.Empty();
}

void USplineRenderComponent::OnRegister() {
	Super::OnRegister();
	RefreshSplineComponent();
}

void USplineRenderComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {
	Super::PostEditChangeProperty(PropertyChangedEvent);
	RefreshSplineComponent();
}

void USplineRenderComponent::DestroyComponent(const bool bPromoteChildren) {
	ClearComponents();
	Super::DestroyComponent(bPromoteChildren);
}


void USplineRenderComponent::InitComponent(USplineMeshComponent* SplinePointMesh, const int Index) {
	SplinePointMesh->SetNetAddressable();
	SplinePointMesh->SetMobility(Mobility);
	SplinePointMesh->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	if (GetWorld() && GetWorld()->bIsWorldInitialized && !SplinePointMesh->IsRegistered())
		SplinePointMesh->RegisterComponent();
	SplinePointMesh->SetVisibility(true);
	SplinePointMesh->SetStaticMesh(Mesh);
	SplinePointMesh->SetForwardAxis(SplineForwardAxis, false);
	SplinePointMesh->SetCollisionEnabled(CollisionType);
	SplinePointMesh->SetCollisionObjectType(CollisionObjectType);
	SplinePointMesh->SetGenerateOverlapEvents(OverlapEvents);
	SplinePointMesh->SetCastShadow(false);
	if (Materials.Num()) {
		for (int v = 0; v < Materials.Num(); v++) {
			SplinePointMesh->SetMaterial(v, Materials[v]);
		}
	}
}

ASplineRenderActor::ASplineRenderActor() {
	PrimaryActorTick.bCanEverTick = false;
	SplineComponent = CreateDefaultSubobject<USplineRenderComponent>(TEXT("Spline Render Component"));
	RootComponent = SplineComponent;
}

void ASplineRenderActor::OnConstruction(const FTransform& Transform) {
	Super::OnConstruction(Transform);
}

