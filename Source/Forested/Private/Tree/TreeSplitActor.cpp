#include "Tree/TreeSplitActor.h"
#include "Tree/TreeChildComponent.h"
#include "Player/FPlayer.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Tree/TreeActor.h"

ATreeSplitActor::ATreeSplitActor() {
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Component"));
	BoxComponent->SetupAttachment(GetRootComponent());
	BoxComponent->SetGenerateOverlapEvents(true);
	BoxComponent->SetMobility(EComponentMobility::Movable);
	BoxComponent->SetCollisionObjectType(ECC_WorldDynamic);
	BoxComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
	BoxComponent->SetHiddenInGame(false);
}

void ATreeSplitActor::OnGameSave_Implementation(UObject* WorldContextObject, UObjectSaveGame* SaveGame) {
	LocationData.Reset();
	for (int i = 0; i < TrunkComponents.Num(); i++) {
		const UTrunkComponent* TrunkComponent = TrunkComponents[i];
		if (IsValid(TrunkComponent)) {
			FObjectData ItemData(TrunkComponent->Item);
			FSplitLocationData SplitLocationData = FSplitLocationData(TrunkComponent->GetClass(), ItemData, TrunkComponent->GetStaticMesh(), TrunkComponent->GetComponentTransform());
			LocationData.Emplace(i, SplitLocationData);
		}
	}
}

void ATreeSplitActor::OnGameLoad_Implementation(UObject* WorldContextObject, UObjectSaveGame* SaveGame) {
	for (const auto& Data : LocationData) {
		if (!Data.Value.ItemData) continue;
		UTreeItem* TreeItem = Data.Value.ItemData.LoadNewObject<UTreeItem>(GetWorld());
		CreateComponent(Data.Value.Class, TreeItem, Data.Value.StaticMesh, Data.Value.Transform);
	}
	AttachComponents();
}

void ATreeSplitActor::Load_Implementation() {
	if (TrunkComponents.Num() < 1) return;
	UTrunkComponent* Last = TrunkComponents.Last();
	/*
	for (const auto& TrunkComponent : TreeChildComponents) {
		if (!TrunkComponent || TrunkComponent->IsAnySimulatingPhysics()) continue;
		TrunkComponent->SetPhysicsLinearVelocity(LinearVelocity);
		TrunkComponent->SetPhysicsAngularVelocityInDegrees(AngularVelocity);
		TrunkComponent->BodyInstance.SetInstanceSimulatePhysics(true, false, true);
	}
	*/
	SetComponentSimulatePhysics(Last, true);
	Last->SetPhysicsLinearVelocity(LinearVelocity);
	Last->SetPhysicsAngularVelocityInDegrees(AngularVelocity);
	LinearVelocity = FVector::ZeroVector;
	AngularVelocity = FVector::ZeroVector;
}

void ATreeSplitActor::Unload_Implementation() {
	if (TrunkComponents.Num() < 1) return;
	UTrunkComponent* Last = TrunkComponents.Last();
	/*
	LinearVelocity = FVector::Zero();
	AngularVelocity = FVector::Zero();
	for (int i = TreeChildComponents.Num() - 1; i >= 0; i--) {
		const auto& TrunkComponent = TreeChildComponents[i];
		if (!TrunkComponent || !TrunkComponent->IsAnySimulatingPhysics()) continue;
		LinearVelocity += TrunkComponent->GetPhysicsLinearVelocity();
		AngularVelocity +=  TrunkComponent->GetPhysicsAngularVelocityInDegrees();
		TrunkComponent->BodyInstance.SetInstanceSimulatePhysics(false, false, true);
	}
	*/
	LinearVelocity = Last->GetPhysicsLinearVelocity();
	AngularVelocity = Last->GetPhysicsAngularVelocityInDegrees();
	SetComponentSimulatePhysics(Last, false);
	//LinearVelocity /= TreeChildComponents.Num();
	//AngularVelocity /= TreeChildComponents.Num();
}

void ATreeSplitActor::Damage_Implementation(AFPlayer* Player, const FHitResult& HitResult, const float Damage, const EDamageType DamageType) {
	if (DamageType == EDamageType::Axe && TrunkComponents.Contains(HitResult.GetComponent())) {
		if (UTrunkComponent* TrunkComponent = Cast<UTrunkComponent>(HitResult.GetComponent())) {
			if (UTrunkComponent* ChildComponent = Cast<UTrunkComponent>(TrunkComponent->GetChildComponent(0))) {
				ChildComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			} else {
				TrunkComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			}
		}
	}
	TArray<UTrunkComponent*> RemoveComponents;
	const int Last = TrunkComponents.Num() - 1;
	for (int i = 0; i < TrunkComponents.Num(); i++) {
		if (UTrunkComponent* TrunkComponent = TrunkComponents[i]) {
			if (RemoveComponents.Contains(TrunkComponent)) continue;
			if (!TrunkComponent->GetAttachParent()) {
				if (i != 0 && i != Last && TrunkComponent->GetChildComponent(0)) {
					ATreeSplitActor* TreeSplitActor = GetWorld()->SpawnActor<ATreeSplitActor>(GetClass(), GetActorTransform());
					TrunkComponent->GetTreeChildComponents(RemoveComponents);
					TreeSplitActor->AddTreeComponents(RemoveComponents);
					continue;
				}
				if ((i == 0 || !TrunkComponent->GetChildComponent(0)) && TrunkComponent->SpawnItemActor()) {
					RemoveComponents.Add(TrunkComponent);
				}
			}
		}
	}
	for (UTrunkComponent* TrunkComponent : RemoveComponents) {
		if (!TrunkComponent) continue;
		TrunkComponents.Remove(TrunkComponent);
		TrunkComponent->DestroyComponent();
	}
	if (TrunkComponents.Num() <= 0) {
		Destroy();
		return;
	}
	UpdateSphereBounds();
}

void ATreeSplitActor::AddTreeComponents(TArray<UTrunkComponent*> InTrunkComponents) {
	for (const UTrunkComponent* TrunkComponent : InTrunkComponents) {
		if (!TrunkComponent) continue;
		CreateComponent(TrunkComponent->GetClass(), TrunkComponent->Item, TrunkComponent->GetStaticMesh(), TrunkComponent->GetComponentTransform());
	}
	AttachComponents();
	
}

UTrunkComponent* ATreeSplitActor::CreateComponent(UClass* Class, UTreeItem* Item, const TSoftObjectPtr<UStaticMesh>& StaticMesh, const FTransform& Transform) {
	if (UTrunkComponent* TrunkComponent = Cast<UTrunkComponent>(AddComponentByClass(Class, true, Transform, false))) {
		TrunkComponent->EnableCollision();
		TrunkComponent->SetGenerateOverlapEvents(false);
		TrunkComponent->Init(Item, StaticMesh);
		TrunkComponent->SetCollisionObjectType(ECC_PhysicsBody);
		TrunkComponent->SetLinearDamping(0.05f);
		TrunkComponent->SetAngularDamping(0.25f);
		TrunkComponent->SetMassOverrideInKg(NAME_None, 50.f);
		TrunkComponent->SetCollisionResponseToChannel(LANDSCAPE_OBJECT_CHANNEL, ECR_Block);
		TrunkComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		TrunkComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		TrunkComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
		TrunkComponent->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
		TrunkComponents.Add(TrunkComponent);
		return TrunkComponent;
	}
	return nullptr;
}

void ATreeSplitActor::AttachComponents() {
	if (!TrunkComponents.Num()) return;
	for (int i = 0; i < TrunkComponents.Num(); i++) {
		UTrunkComponent* TrunkComponent = TrunkComponents[i];
		if (TrunkComponents.IsValidIndex(i + 1))
			TrunkComponent->AttachToComponent(TrunkComponents[i + 1], FAttachmentTransformRules::KeepWorldTransform);
	}
	if (PLAYER) SetComponentSimulatePhysics(TrunkComponents.Last(), PLAYER->IsActorLoaded(this));
	UpdateSphereBounds();
}

void ATreeSplitActor::SetComponentSimulatePhysics(const UPrimitiveComponent* Component, const bool Simulate) {
	if (!Component) return;
	FBodyInstance* BodyInstance = Component->GetBodyInstance();

	if (Simulate && !BodyInstance->bSimulatePhysics) {
		BodyInstance->ApplyWeldOnChildren(); //if setting physics enabled, apply weld
	}
	
	BodyInstance->bSimulatePhysics = Simulate;
	BodyInstance->UpdateInstanceSimulatePhysics();
}

void ATreeSplitActor::UpdateSphereBounds() {
	FBox Box(ForceInit);
	for (const UTrunkComponent* Component : TrunkComponents) {
		if (!Component) continue;
		Box += Component->CalcLocalBounds().GetBox();
	}
	//not this ->
	BoxComponent->AttachToComponent(TrunkComponents[0], FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	BoxComponent->SetRelativeLocation(Box.GetCenter());
	BoxComponent->SetBoxExtent(Box.GetExtent());
}

void ATreeSplitActor::AddImpulse(const FVector& Impulse) {
	for (UTrunkComponent* TrunkComponent : TrunkComponents) {
		if (!TrunkComponent) continue;
		TrunkComponent->AddImpulse(Impulse);
	}
}
