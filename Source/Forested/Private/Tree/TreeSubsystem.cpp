#include "Tree/TreeSubsystem.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Tree/TreeActor.h"
#include "Tree/TreeChildActor.h"

//TODO: dirt movement on creation

AInstanceActorBase::AInstanceActorBase() {
	PrimaryActorTick.bCanEverTick = false;

	InstancedMeshComponent = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("Instanced Mesh Component"));
	RootComponent = InstancedMeshComponent;
	InstancedMeshComponent->SetMobility(EComponentMobility::Stationary);
	
	
	InstancedMeshComponent->NumCustomDataFloats = 3;
	InstancedMeshComponent->SetGenerateOverlapEvents(true);
	InstancedMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	InstancedMeshComponent->SetCollisionResponseToChannel(HIT_TRACE_CHANNEL, ECR_Block);
}

void AInstanceActorBase::Init(UStaticMesh* Mesh) {
	InstancedMeshComponent->SetStaticMesh(Mesh);
}

void AInstanceActorBase::Damage_Implementation(AFPlayer* Player, const FHitResult& HitResult, const float Damage, const EDamageType DamageType) {
	const int i = HitResult.Item;
	if (i < 0) return;
	for (const auto& Pair : Actors) {
		if (Pair.Value == i) {
			UDamageLibrary::ApplyDamage(Pair.Key, Player, HitResult, Damage, DamageType);
			return;
		}
	}
}

int32 AInstanceActorBase::AddActor(AActor* Actor) {
	if (Actors.Contains(Actor)) return -1;
	const FTransform Transform = Actor->GetActorTransform().GetRelativeTransform(InstancedMeshComponent->GetComponentTransform());
	int32 i = InstancedMeshComponent->AddInstance(Transform);
	Actors.Emplace(Actor, i);
	UpdateCustomData(i, Actor);
	return i;
}

int32 AInstanceActorBase::RemoveActor(AActor* Actor) {
	if (!Actors.Contains(Actor)) return -1;
	int32 OutIndex;
	Actors.RemoveAndCopyValue(Actor, OutIndex);
	InstancedMeshComponent->RemoveInstance(OutIndex);
	return OutIndex;
}

void AInstanceActorBase::UpdateActorTransform(const AActor* Actor, const FTransform& Transform) {
	if (!Actors.Contains(Actor)) return;
	const int32 Index = Actors[Actor];
	InstancedMeshComponent->UpdateInstanceTransform(Index, Transform, true);
	UpdateCustomData(Index, Actor);
}

void ATreeChildInstanceActor::UpdateTree(const ATreeActor* TreeActor, const ATreeChildActor* TreeChildActor) {
	FTransform Transform = TreeActor->GetTransform();
	Transform.SetScale3D(FVector(TreeChildActor->GetCurrentSize()));
	UpdateActorTransform(TreeActor, Transform);
}

void ATreeChildInstanceActor::UpdateCustomData(const int32 InstanceIndex, const AActor* Actor) const {
	if (const ATreeActor* TreeActor = Cast<ATreeActor>(Actor)) {
		if (TreeActor->HasTreeChildActor()) {
			InstancedMeshComponent->SetCustomDataValue(InstanceIndex, 0, TreeActor->GetTreeChildActor()->GetStartGrowthTime());
			InstancedMeshComponent->SetCustomDataValue(InstanceIndex, 1, TreeActor->GetTreeChildActor()->GetEndGrowthTime());
			InstancedMeshComponent->SetCustomDataValue(InstanceIndex, 2, TreeActor->GetTreeChildActor()->GetMinSize());
		}
	}
}

bool UTreeSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const {
	//is playing game
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

AInstanceActorBase* UTreeSubsystem::GetTreeInstanceActor_Internal(UStaticMesh* Mesh, const TSubclassOf<AInstanceActorBase> Class) {
	if (!TreeInstanceActors.Contains(Mesh)) {
		AInstanceActorBase* TreeInstanceActor = CastChecked<AInstanceActorBase>(GetWorld()->SpawnActor(Class));
		TreeInstanceActor->Init(Mesh);
		TreeInstanceActors.Emplace(Mesh, TreeInstanceActor);
	}
	return TreeInstanceActors[Mesh];
}

void UTreeSubsystem::AddTree(ATreeActor* TreeActor) {
	if (!TreeActor && HasTreeInstance(TreeActor)) return;
	if (UStaticMesh* StaticMesh = TreeActor->DirtComponent->GetStaticMesh()) {
		GetOrCreateTreeActorInstance(StaticMesh)->AddActor(TreeActor);
	}
}

void UTreeSubsystem::RemoveTree(ATreeActor* TreeActor) {
	if (TreeActor && HasTreeInstance(TreeActor)) {
		if (UStaticMesh* StaticMesh = TreeActor->DirtComponent->GetStaticMesh()) {
			GetOrCreateTreeActorInstance(StaticMesh)->RemoveActor(TreeActor);
		}
	}
}

bool UTreeSubsystem::HasTreeInstance(const ATreeActor* TreeActor) const {
	const UStaticMesh* StaticMesh = TreeActor->DirtComponent->GetStaticMesh();
	if (!StaticMesh) return false;
	return TreeInstanceActors.Contains(StaticMesh) && TreeInstanceActors[StaticMesh]->ContainsActor(TreeActor);
}

void UTreeSubsystem::AddTreeChild(ATreeActor* TreeActor, const ATreeChildActor* TreeChildActor) {
	if (!TreeActor || !TreeChildActor || HasTreeChildInstance(TreeActor, TreeChildActor)) return;
	if (UStaticMesh* TreeInstanceMesh = TreeChildActor->GetTreeInstanceMesh()) {
		GetOrCreateTreeChildInstance(TreeInstanceMesh)->AddActor(TreeActor);
	}
}

void UTreeSubsystem::RemoveTreeChild(ATreeActor* TreeActor, const ATreeChildActor* TreeChildActor) {
	if (!TreeActor || !TreeChildActor || !HasTreeChildInstance(TreeActor, TreeChildActor)) return;
	if (UStaticMesh* TreeInstanceMesh = TreeChildActor->GetTreeInstanceMesh()) {
		GetOrCreateTreeChildInstance(TreeInstanceMesh)->RemoveActor(TreeActor);
	}
}

bool UTreeSubsystem::HasTreeChildInstance(const ATreeActor* TreeActor, const ATreeChildActor* TreeChildActor) const {
	const UStaticMesh* TreeInstanceMesh = TreeChildActor->GetTreeInstanceMesh();
	if (!TreeInstanceMesh) return false;
	return TreeInstanceActors.Contains(TreeInstanceMesh) && TreeInstanceActors[TreeInstanceMesh]->ContainsActor(TreeActor);
}

void UTreeSubsystem::SetInstanceTransform(const ATreeActor* TreeActor, const ATreeChildActor* TreeChildActor) {
	if (!TreeActor || !TreeChildActor || !HasTreeChildInstance(TreeActor, TreeChildActor)) return;
	UStaticMesh* StaticMesh = TreeChildActor->GetTreeInstanceMesh();
	if (!StaticMesh) return;
	GetOrCreateTreeChildInstance(StaticMesh)->UpdateTree(TreeActor, TreeChildActor);
}

void UTreeSubsystem::ResetInstanceTransform(const ATreeActor* TreeActor, const ATreeChildActor* TreeChildActor) {
	if (!TreeActor || !TreeChildActor || !HasTreeChildInstance(TreeActor, TreeChildActor)) return;
	UStaticMesh* StaticMesh = TreeChildActor->GetTreeInstanceMesh();
	if (!StaticMesh) return;
	GetOrCreateTreeChildInstance(StaticMesh)->UpdateActorTransform(TreeActor, TreeActor->GetTransform());
}