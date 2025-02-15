#include "Tree/TreeActor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h" 
#include "Components/WidgetComponent.h"
#include "FPlayer.h"
#include "Engine/AssetManager.h"
#include "Item/ItemActor.h"
#include "Item/PlayerInventory.h"
#include "Item/SeedItem.h"
#include "Tree/TreeSubsystem.h"

ATreeActor::ATreeActor(){
 	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->PrimaryComponentTick.bCanEverTick = false;
	Root->SetMobility(EComponentMobility::Static);
	RootComponent = Root;
	TreeChildActorComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("Tree Child Actor"));
	TreeChildActorComponent->SetMobility(EComponentMobility::Movable);
	TreeChildActorComponent->SetupAttachment(GetRootComponent());
	DirtComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Dirt Mesh"));
	DirtComponent->SetMobility(EComponentMobility::Static);
	DirtComponent->SetupAttachment(GetRootComponent());
	DirtComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DirtComponent->SetGenerateOverlapEvents(false);
	DirtComponent->SetHiddenInGame(true);
	StumpComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Stump Mesh"));
	StumpComponent->SetMobility(EComponentMobility::Static);
	StumpComponent->SetupAttachment(GetRootComponent());
	StumpComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StumpComponent->SetGenerateOverlapEvents(false);
	StumpComponent->SetVisibility(false);
}

void ATreeActor::BeginPlay() {
	Super::BeginPlay();
	SetPosition = GetActorLocation();
	TREE_SUBSYSTEM->AddTree(this);
}

void ATreeActor::Damage_Implementation(AFPlayer* Player, const FHitResult& HitResult, const float Damage, const EDamageType DamageType) {
	switch(DamageType) {
	case EDamageType::Axe:
		if (!HasTreeChildActor() || HitResult.GetComponent() == StumpComponent || HitResult.GetComponent() == DirtComponent)
			break;
		if (GetTreeChildActor()->DamageTree(Player, Damage))
			SetTreeFelled(Player->GetActorRightVector());
		break;
	case EDamageType::Trowel:
		if (HasTreeChildActor() || (HitResult.GetComponent() != StumpComponent && HitResult.GetComponent() != DirtComponent))
			break;
		if (IsStump()) {
			if (const ATreeChildActor* DefaultTreeActor = CastChecked<ATreeChildActor>(TreeChildActorClass.Get()->GetDefaultObject())) {
				UItem* Item = NewObject<UItem>(GetWorld(), DefaultTreeActor->GetLogItemClass());
				AItemActor::SpawnItemActor(GetWorld(), StumpComponent->GetComponentTransform(), Item);
				TreeChildActorClass = nullptr;
			}
			break;
		}
		Health = Health - Damage;
		//SetActorLocation(SetPosition - FVector(0.f, 0.f, 7.5f * (100.f - DirtHealth) / 100.f));
		//InstanceManager->UpdateTreePosition(this, IsSunk());
		break;
	default:
		break;
	}
}

bool ATreeActor::Selected_Implementation(AFPlayer* Player, const FHitResult& HitResult, const float HeldTime) {
	if (HasTreeChildActor() || IsStump()) return false;
	FItemHeap SelectedItem;
	if (!Player->PlayerInventory->GetSelectedItem(SelectedItem) || !SelectedItem->GetClass()->ImplementsInterface(USeedInterface::StaticClass()))
		return false;
	if (UItem* Item = Player->PlayerInventory->RemoveItem(Player->PlayerInventory->GetSelectedSlot())) {
		SetTreeChildActor(ISeedInterface::Execute_GetTreeActorClass(Item));
		ISeedInterface::Execute_OnSeedPlanted(Item, this);
		return true;
	}
	return false;
}

void ATreeActor::OnGameSave_Implementation(UObject* WorldContextObject, UObjectSaveGame* SaveGame) {
	TreeChildData = FObjectData();
	if (ATreeChildActor* TreeActor = GetTreeChildActor()) {
		TreeActor->OnGameSave();
		TreeChildData = FObjectData(TreeActor);
	}
}

void ATreeActor::OnGameLoad_Implementation(UObject* WorldContextObject, UObjectSaveGame* SaveGame) {
	if (TreeChildData) {
		SetTreeChildActor_Internal(TreeChildData.ObjectClass.Get());
		if (ATreeChildActor* TreeActor = GetTreeChildActor()) {
			TreeChildData.LoadObject(GetTreeChildActor());
			TREE_SUBSYSTEM->AddTreeChild(this, GetTreeChildActor());
			if (TreeActor->IsFalling())
				SetTreeFelled(TreeActor->GetTreeFallDirection());
			TreeActor->OnGameLoad();
			TreeChildData = FObjectData();
		}
		return;
	}
	if (!IsStump()) return;
	StumpComponent->SetMobility(EComponentMobility::Movable);
	StumpComponent->SetVisibility(true);
	StumpComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	StumpComponent->SetStaticMesh(TreeChildActorClass.GetDefaultObject()->GetStumpMesh());
	StumpComponent->SetRelativeScale3D(FVector(StumpScale));
}

void ATreeActor::LoadDefaults_Implementation(UObject* WorldContextObject) {
	if (DefaultTreeChildActorClass.IsNull()) {
		ClearTreeChildActor();
		return;
	}
	FSerializationLibrary::LoadAsync(DefaultTreeChildActorClass, [this] {
		SetTreeChildActor_Internal(DefaultTreeChildActorClass.Get());
		GetTreeChildActor()->LoadDefaults();
		TREE_SUBSYSTEM->AddTreeChild(this, GetTreeChildActor());
	});
}

void ATreeActor::LoadTick_Implementation() {
	if (!HasTreeChildActor()) return;
	GetTreeChildActor()->LoadTick();
	TREE_SUBSYSTEM->SetInstanceTransform(this, GetTreeChildActor());
}

void ATreeActor::Load_Implementation() {
	if (!HasTreeChildActor()) return;
	GetTreeChildActor()->Load();
}

void ATreeActor::Unload_Implementation() {
	if (!HasTreeChildActor()) return;
	GetTreeChildActor()->Unload();
	TREE_SUBSYSTEM->ResetInstanceTransform(this, GetTreeChildActor());
}

void ATreeActor::SetTreeFelled(const FVector& FallDirection) {
	if (!HasTreeChildActor()) return;
	TREE_SUBSYSTEM->RemoveTreeChild(this, GetTreeChildActor());
	StumpComponent->SetMobility(EComponentMobility::Movable);
	StumpComponent->SetVisibility(true);
	StumpComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	StumpScale = GetTreeChildActor()->GetCurrentSize();
	StumpComponent->SetStaticMesh(GetTreeChildActor()->GetStumpMesh());
	StumpComponent->SetRelativeScale3D(FVector(StumpScale));
	GetTreeChildActor()->SetTreeFelled(FallDirection);
}

void ATreeActor::TreeSpawned() {
	SetActorLocation(GetActorLocation() - FVector(0.f, 0.f, 50.f));
}

void ATreeActor::ClearTreeChildActor(const bool DestroyStump) {
	if (IsStump() || DestroyStump)
		TreeChildActorClass = nullptr;
	if (!HasTreeChildActor()) return;
	TREE_SUBSYSTEM->RemoveTreeChild(this, GetTreeChildActor());
	TreeChildActorComponent->SetChildActorClass(nullptr);
	TreeChildActor = nullptr;
}

void ATreeActor::SetTreeChildActor(const TSoftClassPtr<ATreeChildActor> InTreeActorClass) {
	if (InTreeActorClass.IsNull() || HasTreeChildActor() || IsStump()) return;
	TSoftClassPtr<ATreeChildActor> SoftTreeChildActorClass = InTreeActorClass;
	FSerializationLibrary::LoadAsync(SoftTreeChildActorClass, [this, SoftTreeChildActorClass] {
		SetTreeChildActor_Internal(SoftTreeChildActorClass.Get());
		TREE_SUBSYSTEM->AddTreeChild(this, GetTreeChildActor());
	});
}

void ATreeActor::SetTreeChildActor_Internal(const TSubclassOf<ATreeChildActor>& InTreeActorClass) {
	TreeChildActorClass = InTreeActorClass;
	TreeChildActorComponent->SetChildActorClass(TreeChildActorClass);
	//TODO: Access Violation -> ChildActorComponent 536 -> UObjectBaseUtility 278
	TreeChildActor = CastChecked<ATreeChildActor>(TreeChildActorComponent->GetChildActor());
	GetTreeChildActor()->Init();
	if (PLAYER->IsActorLoaded(this)) {
		GetTreeChildActor()->Load();
		return;
	}
	GetTreeChildActor()->Unload();
}
