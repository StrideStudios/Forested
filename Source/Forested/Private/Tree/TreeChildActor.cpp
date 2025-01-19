#include "Tree/TreeChildActor.h"
#include "Kismet/KismetMathLibrary.h" 
#include "Kismet/KismetSystemLibrary.h" 
#include "GameFramework/Character.h" 
#include "Components/CapsuleComponent.h" 
#include "Tree/TreeActor.h"
#include "Tree/TreeSplitActor.h"
#include "LevelDefaults.h"
#include "FPlayer.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Forested/Forested.h"

//tree mesh seems to be too large, could be because beginplay?
ATreeChildActor::ATreeChildActor() {
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->SetMobility(EComponentMobility::Static);
	RootComponent = Root;
	TreeRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Tree Root"));
	TreeRoot->SetMobility(EComponentMobility::Static);
	TreeRoot->SetupAttachment(GetRootComponent());
	TreeMovement = CreateDefaultSubobject<USceneComponent>(TEXT("Tree Movement"));
	TreeMovement->SetMobility(EComponentMobility::Static);
	TreeMovement->SetupAttachment(TreeRoot);
	RotatePoint = CreateDefaultSubobject<USceneComponent>(TEXT("Rotate Point"));
	RotatePoint->SetMobility(EComponentMobility::Static);
	RotatePoint->SetupAttachment(TreeMovement);
	TrunkCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Trunk Collision"));
	TrunkCollision->SetMobility(EComponentMobility::Static);
	TrunkCollision->SetupAttachment(RotatePoint);
	TrunkCollision->SetGenerateOverlapEvents(false);
	TrunkCollision->SetCollisionResponseToAllChannels(ECR_Overlap);
	TrunkCollision->SetCollisionObjectType(ECC_WorldDynamic);
	TrunkCollision->SetCollisionResponseToChannel(HIT_TRACE_CHANNEL, ECR_Ignore);
	TrunkCollision->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
	TrunkCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BranchCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Branch Collision"));
	BranchCollision->SetMobility(EComponentMobility::Static);
	BranchCollision->SetupAttachment(RotatePoint);
	BranchCollision->SetGenerateOverlapEvents(false);
	BranchCollision->SetCollisionResponseToAllChannels(ECR_Overlap);
	BranchCollision->SetCollisionObjectType(ECC_WorldDynamic);
	BranchCollision->SetCollisionResponseToChannel(HIT_TRACE_CHANNEL, ECR_Ignore);
	BranchCollision->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
	BranchCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
}

void ATreeChildActor::BeginPlay() {
	Super::BeginPlay();
	TrunkCollision->OnComponentBeginOverlap.AddUniqueDynamic(this, &ATreeChildActor::OnTreeOverlap);
	BranchCollision->OnComponentBeginOverlap.AddUniqueDynamic(this, &ATreeChildActor::OnTreeOverlap);
}

void ATreeChildActor::Tick(const float DeltaTime) {
	Super::Tick(DeltaTime);
	FelledTime += DeltaTime;
	TreeMovement->SetRelativeRotation(UKismetMathLibrary::ComposeRotators(TreeMovement->GetRelativeRotation(), FRotator(TreeFallDirection.X, 0.f, -TreeFallDirection.Y) * FMath::Pow(FelledTime, FallPower) * FallSpeed));
	for (UTreeChildComponent* TreeComponent : GetTreeChildComponents()) {
		if (!TreeComponent) continue;
		TreeComponent->SetCustomPrimitiveDataFloat(1, FMath::Clamp(FelledTime, 0.f, 1.f));
	}
}

bool ATreeChildActor::DamageTree(AFPlayer* Player, const float Damage) {
	if (GetCurrentGrowth() < CanDamageGrowth) return false;
	Health = FMath::Clamp(Health - Damage, 0.f, 100.f);
	return Health <= 0.f;
}

void ATreeChildActor::SetTreeFelled(const FVector& FallDirection) {
	Health = 0.f;
	TreeRoot->SetRelativeScale3D(FVector(GetCurrentSize()));
	Root->SetMobility(EComponentMobility::Movable);
	TreeRoot->SetMobility(EComponentMobility::Movable);
	for (UTreeChildComponent* TreeComponent : GetTreeChildComponents()) {
		if (!TreeComponent) continue;
		TreeComponent->EnableCollision();
		if (UProductComponent* ProductComponent = Cast<UProductComponent>(TreeComponent)) {
			if (ProductComponent->GetCurrentGrowth() <= 0.f) continue;
			ProductComponent->SetCustomPrimitiveDataFloat(0, ProductComponent->GetCurrentGrowth() / 100.f);
			ProductComponent->SetRelativeScale3D(FVector(ProductComponent->GetCurrentSize()));
			continue;
		}
		TreeComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &ATreeChildActor::OnTreeComponentOverlap);
	}
	const float TrunkCapsuleRadius = TrunkCollision->GetScaledCapsuleRadius();
	TreeMovement->SetRelativeLocation(-TreeFallDirection * TrunkCapsuleRadius);
	RotatePoint->SetRelativeLocation(-TreeFallDirection * -TrunkCapsuleRadius);
	TreeMovement->SetMobility(EComponentMobility::Movable);
	RotatePoint->SetMobility(EComponentMobility::Movable);
	BranchCollision->SetGenerateOverlapEvents(true);
	BranchCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BranchCollision->SetMobility(EComponentMobility::Movable);
	BranchCollision->SetCollisionResponseToChannel(HIT_TRACE_CHANNEL, ECR_Ignore);
	TrunkCollision->SetGenerateOverlapEvents(true);
	TrunkCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TrunkCollision->SetMobility(EComponentMobility::Movable);
	TrunkCollision->SetCollisionResponseToChannel(HIT_TRACE_CHANNEL, ECR_Ignore);
	SetActorTickEnabled(true);
	TreeFallDirection = FallDirection;
}

void ATreeChildActor::Init() {
	TArray<USceneComponent*> Components;
	RotatePoint->GetChildrenComponents(false, Components);
	Components.Sort([](const USceneComponent& S1, const USceneComponent& S2) {
		return S1.GetName() > S2.GetName();
	});
	TArray<UTreeChildComponent*> InverseTreeComponents;
	for (USceneComponent* Component : Components) {
		if (UTreeChildComponent* TreeComponent = Cast<UTreeChildComponent>(Component)) {
			TreeComponent->GetTreeChildComponents(InverseTreeComponents);
		}
	}
	for (int i = InverseTreeComponents.Num() - 1; i >= 0; i--) {
		UTreeChildComponent* TreeComponent = InverseTreeComponents[i];
		TreeComponent->Init();
		TreeChildComponents.Add(TreeComponent);
	}
}

void ATreeChildActor::LoadTick() {
	for (const auto& Component : TreeChildComponents) {
		if (!Component) continue;
		if (UProductComponent* ProductComponent = Cast<UProductComponent>(Component)) {
			ProductComponent->SetWorldScale3D(FVector(ProductComponent->GetCurrentSize()));
		}
	}
}

void ATreeChildActor::Load() {
	for (const auto& Component : TreeChildComponents) {
		if (!Component) continue;
		if (UProductComponent* ProductComponent = Cast<UProductComponent>(Component)) {
			ProductComponent->SetHiddenInGame(false);
			ProductComponent->SetWorldScale3D(FVector(ProductComponent->GetCurrentSize()));
		}
	}
}

void ATreeChildActor::Unload() {
	for (const auto& Component : TreeChildComponents) {
		if (!Component) continue;
		if (UProductComponent* ProductComponent = Cast<UProductComponent>(Component)) {
			ProductComponent->SetHiddenInGame(true);
		}
	}
}

void ATreeChildActor::OnGameSave() {
	for (int i = 0; i < TreeChildComponents.Num(); i++) {
		if (const UTreeChildComponent* TreeComponent = TreeChildComponents[i]) {
			if (!TreeComponent->IsA(UProductComponent::StaticClass())) continue;
			FObjectData Data(TreeComponent);
			TreeChildData.Emplace(i, Data);
		}
	}
}
void ATreeChildActor::OnGameLoad() {
	for (int i = 0; i < TreeChildComponents.Num(); i++) {
		if (!TreeChildData.Contains(i)) continue;
		if (UTreeChildComponent* TreeComponent = TreeChildComponents[i])
			TreeChildData[i].LoadObject(TreeComponent);
	}
}

void ATreeChildActor::LoadDefaults() {
	StartGrowthTime = SKY->GetDaytimePassed() - StartGrowth / (100.f / DaysUntilGrown);
	EndGrowthTime = StartGrowthTime + DaysUntilGrown;
	for (UTreeChildComponent* TreeComponent : TreeChildComponents) {
		if (!TreeComponent) continue;
		if (UProductComponent* ProductComponent = Cast<UProductComponent>(TreeComponent)) {
			ProductComponent->Init(EndGrowthTime, EndGrowthTime + ProductComponent->GetDaysUntilGrown(UKismetMathLibrary::RandomFloat()));
		}
	}
}

void ATreeChildActor::OnTreeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if (!IsFalling() || OtherActor == this || OtherActor == GetParentActor() || OtherActor == PLAYER || OtherActor->IsA(AInstanceActorBase::StaticClass())) return;
	Health = 100.f;
	FelledTime = 0.f;
	TreeFallDirection = FVector::ZeroVector;
	TArray<UTrunkComponent*> TrunkComponents;
	for (UTreeChildComponent* TreeComponent : TreeChildComponents) {
		if (!IsValid(TreeComponent) || !TreeComponent->IsRegistered()) continue; //this and overlapcomponent occur at opposite times in which they should (fix by removing from array?)
		if (UTrunkComponent* TrunkComponent = Cast<UTrunkComponent>(TreeComponent)) {
			TrunkComponents.Add(TrunkComponent);
			continue;
		}
		TreeComponent->SpawnItemActor();
		TreeComponent->DestroyComponent();
	}
	ATreeSplitActor* TreeSplitActor = GetWorld()->SpawnActor<ATreeSplitActor>(ATreeSplitActor::StaticClass(), RotatePoint->GetComponentTransform());
	TreeSplitActor->AddTreeComponents(TrunkComponents);
	//TreeSplitActor->AddImpulse(FVector(0.f, 0.f, -1000.f));
	if (ATreeActor* TreeActor = Cast<ATreeActor>(GetParentActor()))
		TreeActor->ClearTreeChildActor();
	//for (int i = GetTrunkComponents().Num() - 1; i >= 0; i--)
	//AddComponentCollision(GetTrunkComponents()[i], true);
	//GetTrunkComponents()[0]->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
}

void ATreeChildActor::OnTreeComponentOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if (!IsFalling() || OtherActor == this || OtherActor == GetParentActor() || OtherActor->IsA(AInstanceActorBase::StaticClass())) return;
	UBranchComponent* BranchComponent = Cast<UBranchComponent>(OverlappedComponent); 
	if (BranchComponent && BranchComponent->IsRegistered()) {
		if (OtherComp->IsPhysicsCollisionEnabled() && BranchComponent->SpawnItemActor()) {
			TreeChildComponents.Remove(BranchComponent);
			BranchComponent->DestroyComponent();
		}
	}
}

float ATreeChildActor::GetCurrentSize() const {
	return UKismetMathLibrary::MapRangeClamped(SKY->GetDaytimePassed(), StartGrowthTime, EndGrowthTime, MinTreeSize, 1.f);
}

float ATreeChildActor::GetCurrentGrowth() const {
	return UKismetMathLibrary::MapRangeClamped(SKY->GetDaytimePassed(), StartGrowthTime, EndGrowthTime, 0.f, 100.f);
}