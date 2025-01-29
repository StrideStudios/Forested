#include "Item/ItemActor.h"
#include "CullStaticMeshComponent.h"
#include "Forested/Forested.h"
#include "Item/PlayerInventory.h"
#include "FPlayer.h"
#include "LevelDefaults.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/AssetManager.h"
#include "Item/Item.h"
#include "Kismet/KismetMathLibrary.h"
#include "SerializationLibrary.h"

/*
 * If you need smoother physics, try substepping with high solver iterations 32-64
 * use async physics then it also runs on its own thread with its own fixed tick rate
 * velocity iterations 2
 * Collision Margin Fraction - with value as small as possible like 0.001
 * Collision Margin Max - value around 0,05 (or less)
 * Collision Max Push Out Velocity - with value about 700
 */
AItemActor::AItemActor() {
	PrimaryActorTick.bCanEverTick = false;
	
	StaticMeshComponent = CreateDefaultSubobject<UCullStaticMeshComponent>(TEXT("Static Mesh Component"));
	RootComponent = StaticMeshComponent;
	StaticMeshComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	StaticMeshComponent->SetMobility(EComponentMobility::Movable);
	StaticMeshComponent->SetGenerateOverlapEvents(false);
	StaticMeshComponent->SetCollisionObjectType(ECC_PhysicsBody);
	StaticMeshComponent->SetCollisionResponseToChannel(LANDSCAPE_OBJECT_CHANNEL, ECR_Block);
	StaticMeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	StaticMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	StaticMeshComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
	StaticMeshComponent->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Component"));
	SphereComponent->SetupAttachment(StaticMeshComponent);
	SphereComponent->SetMassScale(NAME_None, 0.f);
	SphereComponent->SetGenerateOverlapEvents(true);
	SphereComponent->SetMobility(EComponentMobility::Movable);
	SphereComponent->SetCollisionObjectType(ECC_PhysicsBody);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
	SphereComponent->SetHiddenInGame(true);

}

void AItemActor::BeginPlay() {
	Super::BeginPlay();
	if (!Item) {
		LOG_ERROR("Requested Item on Item Actor %s did not exist", *GetName());
		Destroy();
		return;
	}

	//tell the item it has been added to the item actor
	IItemActorInterface::Execute_OnAddedToItemActor(Item, this);
	
	const FBoxSphereBounds Bounds = StaticMeshComponent->GetBounds();
	SphereComponent->SetSphereRadius(Bounds.SphereRadius);
	SphereComponent->SetRelativeLocation(Bounds.Origin);
	//TODO: bool with set override mesh and static function in library would be better for blueprint readability
	const FItemActorProperties Properties = IItemActorInterface::Execute_GetItemActorProperties(Item);
	StaticMeshComponent->SetCullScale(Properties.CullScale);
	StaticMeshComponent->SetMassOverrideInKg(NAME_None, Properties.MassInKgOverride, Properties.bOverrideMass);
	StaticMeshComponent->SetLinearDamping(Properties.LinearDrag);
	StaticMeshComponent->SetAngularDamping(Properties.AngularDrag);
	
	if (!Properties.bOverrideStaticMesh) {
		StaticMesh = StaticMeshComponent->GetStaticMesh();
		if (!StaticMesh) {
			LOG_ERROR("Item Actor with Item %s has no static mesh", *GetItem()->GetItemName().ToString());
			Destroy();
			return;
		}
		Init();
		return;
	}
	
	TSoftObjectPtr<UStaticMesh> SoftStaticMesh = Properties.StaticMesh;
	FSerializationLibrary::LoadAsync(SoftStaticMesh, [this, SoftStaticMesh] {
		StaticMesh = SoftStaticMesh.Get();
		StaticMeshComponent->SetStaticMesh(StaticMesh);
		Init();
	});
}

void AItemActor::Init() {
	const FBoxSphereBounds Bounds = StaticMeshComponent->GetBounds();
	SphereComponent->SetSphereRadius(Bounds.SphereRadius);
	SphereComponent->SetRelativeLocation(Bounds.Origin);
	ReceiveInit();
	if (PLAYER->IsActorLoaded(this)) {
		Execute_Load(this);
	} else {
		Execute_Unload(this);
	}
	OnItemLoadComplete.ExecuteIfBound(this);
}

bool AItemActor::Selected_Implementation(AFPlayer* Player, const FHitResult& HitResult, const float HeldTime) {
	if (!Item) {
		LOG_ERROR("Requested Item on Item Actor %s did not exist", *GetName());
		Destroy();
		return false;
	}
	if (Player->PlayerInventory->AddItem(Item) < 0) return false;
	Destroy();
	return true;
}

void AItemActor::OnGameSave_Implementation(UObject* WorldContextObject, UObjectSaveGame* SaveGame) {
	if (!Item) {
		LOG_ERROR("Item Actor %s Without Item cannot save", *GetName());
		return;
	}
	ItemData = FObjectData(Item);
}

void AItemActor::OnGameLoad_Implementation(UObject* WorldContextObject, UObjectSaveGame* SaveGame) {
	if (!ItemData) {
		LOG_ERROR("Loaded Item Class on Item Actor %s does not exist", *GetName());
		Destroy();
		return;
	}
	Item = ItemData.LoadNewObject<UItem>(GetWorld());
}

void AItemActor::Load_Implementation() {
	if (!StaticMesh || StaticMeshComponent->IsAnySimulatingPhysics()) return;
	StaticMeshComponent->SetSimulatePhysics(true);
	StaticMeshComponent->SetPhysicsLinearVelocity(LinearVelocity);
	StaticMeshComponent->SetPhysicsAngularVelocityInDegrees(AngularVelocity);
	LinearVelocity = FVector::ZeroVector;
	AngularVelocity = FVector::ZeroVector;
}

void AItemActor::Unload_Implementation() {
	if (!StaticMesh || !StaticMeshComponent->IsAnySimulatingPhysics()) return;
	LinearVelocity = StaticMeshComponent->GetPhysicsLinearVelocity();
	AngularVelocity = StaticMeshComponent->GetPhysicsAngularVelocityInDegrees();
	StaticMeshComponent->SetSimulatePhysics(false);
}

bool AItemActor::SpawnItemActor_Internal(const UObject* WorldContextObject, const FTransform& Transform, UItem* Item, FItemLoadDelegate OnItemLoadComplete, const FVector InLinearVelocity, const FVector InAngularVelocity) {
	UWorld* World = WorldContextObject->GetWorld();
	if (!World) {
		LOG_ERROR("Invalid World Given");
		return false;
	}
	if (!Item) {
		LOG_ERROR("Input Item is not valid");
		return false;
	}
	if (!Item->GetClass()->ImplementsInterface(UItemActorInterface::StaticClass())) {
		LOG_ERROR("Item %s must have the Item Actor Interface Implemented for it to be spawned in world", *Item->GetName());
		return false;
	}
	const TSoftClassPtr<AItemActor> Class = IItemActorInterface::Execute_GetItemActorProperties(Item).ItemActorClass;
	if (Class.IsNull()) {
		LOG_ERROR("Item %s with Item Actor Interface must have non-null item actor class", *Item->GetName());
		return false;
	}

	//load the class to be used when spawned
	FSerializationLibrary::LoadAsync(Class, [World, Class, Transform, Item, InLinearVelocity, InAngularVelocity, OnItemLoadComplete]{
		AItemActor* ItemActor = World->SpawnActorDeferred<AItemActor>(TSubclassOf<AItemActor>(Class.Get()), Transform);
		ItemActor->Item = Item;
		ItemActor->LinearVelocity = InLinearVelocity;
		ItemActor->AngularVelocity = InAngularVelocity;
		ItemActor->OnItemLoadComplete = OnItemLoadComplete;
		ItemActor->FinishSpawning(Transform);
	});
	return true;
}

FItemActorProperties UItemActorSpawnAsyncAction::SetOverrideStaticMesh(FItemActorProperties& Properties, const TSoftObjectPtr<UStaticMesh> StaticMesh) {
	if (!StaticMesh.IsNull()) {
		Properties.bOverrideStaticMesh = true;
		Properties.StaticMesh = StaticMesh;
	}
	return Properties;
}

FItemActorProperties UItemActorSpawnAsyncAction::SetOverrideMass(FItemActorProperties& Properties, const float MassInKgOverride) {
	Properties.bOverrideMass = true;
	Properties.MassInKgOverride = MassInKgOverride;
	return Properties;
}

UItemActorSpawnAsyncAction* UItemActorSpawnAsyncAction::SpawnItemActor(UObject* WorldContextObject, const FTransform Transform, UItem* Item, const FVector InLinearVelocity, const FVector InAngularVelocity) {
	UItemActorSpawnAsyncAction* Node = NewObject<UItemActorSpawnAsyncAction>();
	Node->Object = WorldContextObject;
	Node->Transform = Transform;
	Node->Item = Item;
	Node->LinearVelocity = InLinearVelocity;
	Node->AngularVelocity = InAngularVelocity;
	return Node;
}

void UItemActorSpawnAsyncAction::Activate() {
	const bool bSuccess = AItemActor::SpawnItemActor(Object, Transform, Item, [this](AItemActor* SpawnedItemActor){
		Complete.Broadcast(SpawnedItemActor);
	}, LinearVelocity, AngularVelocity);
	if (!bSuccess) Failed.Broadcast(nullptr);
}

