#include "Player/PlayerInventoryActor.h"
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

void APlayerInventoryActor::Init() {
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

void APlayerInventoryActor::SetupRootAttachment() const {
	if (AttachSocket == NAME_None) return;
	UChildActorComponent* Component = GetParentComponent();
	if (Component && Component->GetAttachParent()) {
		if (USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(Component->GetAttachParent())) {
			Component->AttachToComponent(SkeletalMeshComponent, FAttachmentTransformRules::KeepRelativeTransform, SkeletalMeshComponent->DoesSocketExist(AttachSocket) ? AttachSocket : NAME_None);
		}
	}
}

UPlayerAnimInstance* APlayerInventoryActor::GetPlayerAnimInstance() {
	return CastChecked<UPlayerAnimInstance>(PLAYER->GetMesh()->GetAnimInstance());
}

bool APlayerInventoryActor::StartMontage(UAnimMontage* MontageToPlay, const float PlayRate, const float StartingPosition) const {
	if (!bAnimationsLoaded || !MontageToPlay || !CanMontagePlay(MontageToPlay, PlayRate, StartingPosition)) return false;
	const float f = GetPlayerAnimInstance()->Montage_Play(MontageToPlay, PlayRate, EMontagePlayReturnType::MontageLength, StartingPosition);
	return f > 0.f;
}

bool APlayerInventoryActor::PauseMontage(const UAnimMontage* Montage) const {
	if (!bAnimationsLoaded) return false;
	FAnimMontageInstance* ActiveMontage = GetPlayerAnimInstance()->GetMontageInstance(Montage, true); 
	if (ActiveMontage && CanMontagePause(ActiveMontage->Montage)) {
		ActiveMontage->Pause();
		return true;
	}
	return false;
}

bool APlayerInventoryActor::ResumeMontage(const UAnimMontage* Montage) const {
	if (!bAnimationsLoaded) return false;
	FAnimMontageInstance* ActiveMontage = GetPlayerAnimInstance()->GetMontageInstance(Montage, true); 
	if (ActiveMontage && !ActiveMontage->IsPlaying() && CanMontageResume(ActiveMontage->Montage)) {
		ActiveMontage->SetPlaying(true);
		return true;
	}
	return false;
}
//TODO: library and parity between this and player inventory
bool APlayerInventoryActor::StopMontage(const float BlendOutTime, const UAnimMontage* Montage) const {
	if (!CanMontageStop(BlendOutTime, Montage)) return false;
	FAlphaBlend Blend = FAlphaBlend(Montage->BlendOut);
	Blend.SetBlendTime(BlendOutTime);
	return StopMontage(Blend, Montage);
}

bool APlayerInventoryActor::StopMontage(const FAlphaBlend& Blend, const UAnimMontage* Montage) const {
	if (!bAnimationsLoaded) return false;
	if (FAnimMontageInstance* ActiveMontage = GetPlayerAnimInstance()->GetMontageInstance(Montage, true)) {
		ActiveMontage->Stop(Blend);
		return true;
	}
	return false;
}
//TODO: Can switch during montage for some reason
void APlayerInventoryActor::OnLeftInteract() {
	ReceiveOnLeftInteract(GetPlayerAnimInstance()->IsAMontageActive());
}

void APlayerInventoryActor::OnRightInteract() {
	ReceiveOnRightInteract(GetPlayerAnimInstance()->IsAMontageActive());
}

void APlayerInventoryActor::OnRightEndInteract() {
	ReceiveOnRightEndInteract(GetPlayerAnimInstance()->IsAMontageActive());
}

void APlayerInventoryActor::OnButtonInteract() {
	ReceiveOnButtonInteract(GetPlayerAnimInstance()->IsAMontageActive());
}

bool APlayerInventoryActor::CanMontagePlay_Implementation(const UAnimMontage* Montage, float PlayRate, float StartingPosition) const {
	return !GetPlayerAnimInstance()->IsAMontageActive();
}

bool APlayerInventoryActor::CanMontagePause_Implementation(const UAnimMontage* Montage) const {
	return GetPlayerAnimInstance()->IsMontageActive(Montage);
}

bool APlayerInventoryActor::CanMontageResume_Implementation(const UAnimMontage* Montage) const {
	return GetPlayerAnimInstance()->IsMontageActive(Montage) && !GetPlayerAnimInstance()->IsMontagePlaying(Montage);
}

bool APlayerInventoryActor::CanMontageStop_Implementation(float BlendOutTime, const UAnimMontage* Montage) const {
	return GetPlayerAnimInstance()->IsMontageActive(Montage);
}

bool APlayerInventoryActor::HasItem() {
	return PLAYER_INVENTORY->HasSelectedItem();
}

bool APlayerInventoryActor::GetItem(FItemHeap& OutItem) {
	return PLAYER_INVENTORY->GetSelectedItem(OutItem);
}