#include "Player/PlayerInventoryActor.h"

#include "MontageLibrary.h"
#include "Player/FPlayer.h"
#include "Player/ViewmodelMeshes.h"
#include "Player/PlayerInventory.h"
#include "Serialization/SerializationLibrary.h"

APlayerInventoryActor::APlayerInventoryActor() {
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	StaticMeshComponent = CreateDefaultSubobject<UViewmodelStaticMeshComponent>(TEXT("Static Mesh Component"));
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticMeshComponent->bSelfShadowOnly = true;
	StaticMeshComponent->SetupAttachment(GetRootComponent());
	SetupRootAttachment();
}

void APlayerInventoryActor::Init(AFPlayer* InPlayer) {
	Player = InPlayer;
	
	SetupRootAttachment();
	bAnimationsLoaded = true;
	ReceiveInit();

	//if a static mesh is defined in the interface, it overrides the mesh used in the hand actor
	//this is useful if the default inventory render actor is all that's needed
	FItemHeap Item;
	if (GetItem(Item)) {
		TSoftClassPtr<APlayerInventoryActor> InventoryRenderClass; TSoftObjectPtr<UStaticMesh> StaticMesh;
		IPlayerInventoryInterface::Execute_GetPlayerInventoryProperties(Item, InventoryRenderClass, StaticMesh);
		if (!StaticMesh.IsNull()) {
			FSerializationLibrary::LoadAsync(StaticMesh, [this, StaticMesh] {
				StaticMeshComponent->SetStaticMesh(StaticMesh.Get());
			});
		}
	}
}

void APlayerInventoryActor::Deinit() {
	ReceiveDeinit();
}

void APlayerInventoryActor::SetupRootAttachment() const {
	if (AttachSocket == NAME_None) return;
	UChildActorComponent* Component = GetParentComponent();
	if (Component && Component->GetAttachParent()) {
		if (USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(Component->GetAttachParent())) {
			Component->AttachToComponent(SkeletalMeshComponent, FAttachmentTransformRules::KeepRelativeTransform, SkeletalMeshComponent->DoesSocketExist(AttachSocket) ? AttachSocket : NAME_None);
		}
	}
}

UPlayerAnimInstance* APlayerInventoryActor::GetPlayerAnimInstance() const {
	return CastChecked<UPlayerAnimInstance>(Player->GetMesh()->GetAnimInstance());
}

void APlayerInventoryActor::OnMontageNotifyBegin(const UAnimMontage* Montage, const FName Notify) {
	//pause montage can be built in because it is common
	if (Notify == "PauseMontage") {
		UMontageLibrary::PauseMontage(GetPlayer()->GetMesh(), Montage);
	}
	ReceiveOnMontageNotifyBegin(Montage, Notify);
}

void APlayerInventoryActor::OnLeftInteract() {
	ReceiveOnLeftInteract(UMontageLibrary::IsAMontageActive(GetPlayerAnimInstance()));
}

void APlayerInventoryActor::OnLeftEndInteract() {
	ReceiveOnLeftEndInteract(UMontageLibrary::IsAMontageActive(GetPlayerAnimInstance()));
}

void APlayerInventoryActor::OnRightInteract() {
	ReceiveOnRightInteract(UMontageLibrary::IsAMontageActive(GetPlayerAnimInstance()));
}

void APlayerInventoryActor::OnRightEndInteract() {
	ReceiveOnRightEndInteract(UMontageLibrary::IsAMontageActive(GetPlayerAnimInstance()));
}

void APlayerInventoryActor::OnButtonInteract() {
	ReceiveOnButtonInteract(UMontageLibrary::IsAMontageActive(GetPlayerAnimInstance()));
}

void APlayerInventoryActor::OnReload() {
	ReceiveOnReload(UMontageLibrary::IsAMontageActive(GetPlayerAnimInstance()));
}

void APlayerInventoryActor::OnEndReload() {
	ReceiveOnEndReload(UMontageLibrary::IsAMontageActive(GetPlayerAnimInstance()));
}

bool APlayerInventoryActor::HasItem() const {
	return GetPlayer()->PlayerInventory->HasSelectedItem();
}

bool APlayerInventoryActor::GetItem(FItemHeap& OutItem) const {
	return GetPlayer()->PlayerInventory->GetSelectedItem(OutItem);
}
