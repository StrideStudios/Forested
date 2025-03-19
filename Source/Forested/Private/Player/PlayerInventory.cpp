#include "Player/PlayerInventory.h"

#include "MontageLibrary.h"
#include "Player/FPlayer.h"
#include "Player/PlayerInputComponent.h"
#include "Widget/PlayerWidget.h"
#include "GameFramework/CharacterMovementComponent.h" 
#include "Items/Item.h"
#include "Camera/CameraComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Items/Inventory.h"
#include "Player/PlayerHud.h"
#include "Tree/TreeActor.h"
#include "Serialization/SerializationLibrary.h"
#include "Player/ViewmodelMeshes.h"
#include "Player/PlayerInventoryActor.h"

UPlayerInventory::UPlayerInventory() {
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UPlayerInventory::BeginPlay() {
	Super::BeginPlay();
	
	RegisterItem();
}

void UPlayerInventory::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (APlayerInventoryActor* RenderActor = Cast<APlayerInventoryActor>(Player->ItemMesh->GetChildActor())) {
		RenderActor->InventoryTick(DeltaTime);
	}
}

//TODO: can interact while montage playing
bool UPlayerInventory::ClearSelectedItem(const float PlayRate) {
	if (const UPlayerAnimInstance* PlayerAnimInstance = Cast<UPlayerAnimInstance>(Player->GetMesh()->GetAnimInstance())) {
		if (!PlayerAnimInstance->CanSwitchItems()) return false;
		SelectedSlot -= GetCapacity();
		RegisterItem(PlayRate);
	}
	return !IsSelectedSlotValid();
}

bool UPlayerInventory::ResetSelectedItem(const float PlayRate) {
	if (SelectedSlot < 0) {
		SelectedSlot += GetCapacity();
		RegisterItem(PlayRate);
	}
	return IsSelectedSlotValid();
}

void UPlayerInventory::ChangeItem(const int Change) {
	if (!IsSelectedSlotValid()) return;
	int NewSlot;
	if (SelectedSlot + Change >= GetCapacity()) {
		NewSlot = 0;
	} else if (SelectedSlot + Change < 0) {
		NewSlot = GetCapacity() - 1;
	} else {
		NewSlot = SelectedSlot + Change;
	}
	SetSelectedSlot_Internal(NewSlot);
	RegisterItem();
}

/*TODO: smoother swimming
swimming transition via unequip
no equipping/movement while swimming
crouching doesnt work + no swim-crouching
falling into water anim
left/right swim anim
 */

void UPlayerInventory::LeftInteract() const {
	if (APlayerInventoryActor* InventoryRenderActor = Cast<APlayerInventoryActor>(Player->ItemMesh->GetChildActor())) {
		InventoryRenderActor->OnLeftInteract();
	}
}

void UPlayerInventory::EndLeftInteract() const {
	if (APlayerInventoryActor* InventoryRenderActor = Cast<APlayerInventoryActor>(Player->ItemMesh->GetChildActor())) {
		InventoryRenderActor->OnLeftEndInteract();
	}
}

void UPlayerInventory::RightInteract() const {
	if (APlayerInventoryActor* InventoryRenderActor = Cast<APlayerInventoryActor>(Player->ItemMesh->GetChildActor())) {
		InventoryRenderActor->OnRightInteract();
	}
}

void UPlayerInventory::EndRightInteract() const {
	if (APlayerInventoryActor* InventoryRenderActor = Cast<APlayerInventoryActor>(Player->ItemMesh->GetChildActor())) {
		InventoryRenderActor->OnRightEndInteract();
	}
}

void UPlayerInventory::ButtonInteract() const {
	if (APlayerInventoryActor* InventoryRenderActor = Cast<APlayerInventoryActor>(Player->ItemMesh->GetChildActor())) {
		InventoryRenderActor->OnButtonInteract();
	}
}

void UPlayerInventory::Reload() const {
	if (APlayerInventoryActor* InventoryRenderActor = Cast<APlayerInventoryActor>(Player->ItemMesh->GetChildActor())) {
		InventoryRenderActor->OnReload();
	}
}

void UPlayerInventory::EndReload() const {
	if (APlayerInventoryActor* InventoryRenderActor = Cast<APlayerInventoryActor>(Player->ItemMesh->GetChildActor())) {
		InventoryRenderActor->OnEndReload();
	}
}

void UPlayerInventory::SetSelectedSlot_Internal(const int Slot) {
	if (const UPlayerAnimInstance* PlayerAnimInstance = Cast<UPlayerAnimInstance>(Player->GetMesh()->GetAnimInstance())) {
		if (!PlayerAnimInstance->CanSwitchItems()) return;
		if (Player->GetHud() && !Player->GetHud()->IsInMenu()) {
			Player->GetHud()->GetInventoryWidget()->SetSlotSelected(SelectedSlot, false);
			Player->GetHud()->GetInventoryWidget()->SetSlotSelected(Slot, true);
		}
		SelectedSlot = FMath::Clamp(Slot, 0, GetCapacity());
	}
}

void UPlayerInventory::RegisterItem(const float PlayRate) const {
	if (UPlayerAnimInstance* PlayerAnimInstance = Cast<UPlayerAnimInstance>(Player->GetMesh()->GetAnimInstance())) {
		FItemHeap SelectedItem; 
		if (IsSelectedSlotValid() && GetSelectedItem(SelectedItem) && SelectedItem) {
			TSoftClassPtr<APlayerInventoryActor> InventoryRenderClass; TSoftObjectPtr<UStaticMesh> StaticMesh;
			IPlayerInventoryInterface::Execute_GetPlayerInventoryProperties(SelectedItem, InventoryRenderClass, StaticMesh);
			//TODO: ASSUMING THIS ISNT LOADING PROPERLY UPON NEW GAME (CLOSE AND OPEN EDITOR)
			//TODO: equipping only on first startup? things not unloading?
			FSerializationLibrary::LoadSync(InventoryRenderClass); //TODO: Don't load sync forever please
			PlayerAnimInstance->SetNewInventoryRenderActor(InventoryRenderClass.Get(), PlayRate);
			return;
		}
		if (!PlayerAnimInstance->GetInventoryRenderActor() || PlayerAnimInstance->GetInventoryRenderActor()->GetClass() != DefaultInventoryRenderActor) {
			PlayerAnimInstance->SetNewInventoryRenderActor(DefaultInventoryRenderActor, PlayRate);
		}
	}
}

bool UPlayerInventory::CanItemBeInserted(const FItemHeap Item, const int Slot) {
	if (!Super::CanItemBeInserted(Item, Slot)) return false;
	if (!Item->GetClass()->ImplementsInterface(UPlayerInventoryInterface::StaticClass())) {
		LOG_EDITOR_WARNING("Item %s could not be added to inventory; It does not implement the player inventory interface", *Item->GetItemName().ToString())
		return false;
	}
	return IsSelectedSlotValid();
}

bool UPlayerInventory::CanItemBeRemoved(const int Slot) {
	return Super::CanItemBeRemoved(Slot) && IsSelectedSlotValid();
}

void UPlayerInventory::OnInsertItem(const FItemHeap Item, const int Slot) {
	if (GetSelectedSlot() != Slot) {
		SetSelectedSlot_Internal(Slot);
	}
	if (Player->GetHud() && !Player->GetHud()->IsInMenu()) {
		Player->GetHud()->GetInventoryWidget()->UpdateSlot(Slot);
	}
	RegisterItem();

	//tell the item it has been added to the player's inventory
	IPlayerInventoryInterface::Execute_OnAddedToPlayerInventory(Item, this);
	
	Super::OnInsertItem(Item, Slot);
}

void UPlayerInventory::OnRemoveItem(const FItemHeap Item, const int Slot) {
	if (GetSelectedSlot() == Slot) {
		RegisterItem();
	}

	if (Player->GetHud() && !Player->GetHud()->IsInMenu()) {
		Player->GetHud()->GetInventoryWidget()->UpdateSlot(Slot);
	}
	
	Super::OnRemoveItem(Item, Slot);
}

void UPlayerInventory::OnAppendItems(const TMap<int32, FItemHeap>& AppendedItems) {
	const int Slot = AppendedItems.Num() ? AppendedItems.end().Key() : 0;
	SetSelectedSlot_Internal(Slot);

	for (auto& Pair : AppendedItems) {
		//update slot of the appended items
		if (Player->GetHud() && !Player->GetHud()->IsInMenu()) {
			Player->GetHud()->GetInventoryWidget()->UpdateSlot(Pair.Key);
		}
		//tell the item it has been added to the player's inventory
		IPlayerInventoryInterface::Execute_OnAddedToPlayerInventory(Pair.Value, this);
	}
	
	RegisterItem();
	
	Super::OnAppendItems(AppendedItems);
}

void UPlayerInventory::Init_Implementation(UObject* WorldContextObject) {
	Player = CastChecked<AFPlayer>(GetOwner());
}

void UPlayerInventory::LoadDefaults_Implementation(UObject* WorldContextObject) {
	TArray<FItemHeap> OutItems;
	AppendItems(DefaultItems, OutItems);
	SetSelectedSlot_Internal(0);
}

bool UPlayerAnimInstance::CanSwitchItems() const {
	return (!RenderActor || RenderActor->CanSwitchItems()) && (!GetCurrentActiveMontage() || GetCurrentActiveMontage() == RenderActor->EquipAnimMontage || GetCurrentActiveMontage() == RenderActor->UnEquipAnimMontage);
}

void UPlayerAnimInstance::Init_Implementation(UObject* WorldContextObject) {
	Player = CastChecked<AFPlayer>(TryGetPawnOwner());
}

void UPlayerAnimInstance::NativeBeginPlay() {
	OnMontageEnded.AddUniqueDynamic(this, &UPlayerAnimInstance::MontageEnded);
	OnMontageBlendingOut.AddUniqueDynamic(this, &UPlayerAnimInstance::MontageBlendOut);
	OnPlayMontageNotifyBegin.AddUniqueDynamic(this, &UPlayerAnimInstance::MontageNotifyBegin);
	OnPlayMontageNotifyEnd.AddUniqueDynamic(this, &UPlayerAnimInstance::MontageNotifyEnd);
}

void UPlayerAnimInstance::NativeUpdateAnimation(const float DeltaSeconds) {
	if (!Player) return;
	PoseBlend = FMath::FInterpTo(PoseBlend, TargetPoseBlend, DeltaSeconds, 10.f);
	Velocity = Player->GetVelocity();
	Player->PlayerInputComponent->GetMovementVector(RightMovement, ForwardMovement);
	ForwardVelocity = Player->GetForwardVelocity();
	RightVelocity = Player->GetRightVelocity();
	CameraRotation = Player->Camera->GetComponentRotation();
	UnitVelocity = Velocity.Size();
	UnitVelocity2D = Velocity.Size2D();
	VerticalVelocity = Velocity.Z;
	IsSwimming = Player->PlayerInputComponent->IsSwimming();
	IsFalling = Player->GetCharacterMovement()->IsFalling() && !IsSwimming;
	IsMoving = !Player->GetCharacterMovement()->GetCurrentAcceleration().Equals(FVector(0.f)) && UnitVelocity > 5.f;
	if (const FAnimMontageInstance* Montage = UMontageLibrary::GetAnyActiveMontageInstance(this)) {
		if (!Montage->bEnableAutoBlendOut && !UMontageLibrary::IsMontageInstancePlaying(Montage)) {
			//when the unequip finishes, we load data for the next actor
			//TODO: move to blend out?
			if (Montage->Montage == RenderActor->UnEquipAnimMontage) {
				LoadAnimationData();
				return;
			}
			if (RenderActor) {
				RenderActor->OnManualBlendOutMontageComplete(Montage->Montage);
			}
		}
	}
}

bool UPlayerAnimInstance::HandleNotify(const FAnimNotifyEvent& AnimNotifyEvent) {
	if (AnimNotifyEvent.NotifyName == "PlayFootstep") {
		Player->PlayFootstep();
		return true;
	}
	if (AnimNotifyEvent.NotifyName == "SwimStart") {
		Player->PlayerInputComponent->bSwimCanMove = true;
		return true;
	}
	if (AnimNotifyEvent.NotifyName == "SwimEnd") {
		Player->PlayerInputComponent->bSwimCanMove = false;
		return true;
	}
	//TODO: this probably
	if (AnimNotifyEvent.NotifyName == "AnimPause") {
		return false;
	}
	return false;
}

void UPlayerAnimInstance::SetNewInventoryRenderActor(const TSubclassOf<APlayerInventoryActor>& InRenderActor, const float PlayRate) {
	if (!InRenderActor) {
		LOG_ERROR("Null Render Actor Given To Anim Instance");
		return;
	}
	NextRenderActor = InRenderActor;
	CurrentPlayRate = PlayRate;

	//if there is no actor equipped or no anim, just load the data
	if (!RenderActor || !RenderActor->UnEquipAnimMontage) {
		LoadAnimationData();
		return;
	}

	//if no montage is active unequip and prepare for loading data
	if (!UMontageLibrary::IsAMontageActive(this)) {
		UMontageLibrary::StartMontage(Player->GetMesh(), RenderActor->UnEquipAnimMontage, CurrentPlayRate);
		return;
	}

	//if equipping and the new render actor is NOT the same, un-equip
	const FAlphaBlend Blend = FAlphaBlend(0.25f);
	FAnimMontageInstance* EquipMontage = UMontageLibrary::GetActiveMontageInstance(this, RenderActor->EquipAnimMontage, true);
	if (EquipMontage && RenderActor->GetClass() != NextRenderActor) {
		EquipMontage->Stop(Blend);
		Montage_Play(RenderActor->UnEquipAnimMontage, CurrentPlayRate);
		return;
	}
	
	//if un-equipping and the new render actor IS the same, re-equip
	FAnimMontageInstance* UnequipMontage = UMontageLibrary::GetActiveMontageInstance(this, RenderActor->UnEquipAnimMontage, true);
	if (UnequipMontage && RenderActor->GetClass() == NextRenderActor) {
		UnequipMontage->Stop(Blend);
		Montage_Play(RenderActor->EquipAnimMontage, CurrentPlayRate);
	}
}

void UPlayerAnimInstance::LoadAnimationData() {
	
	//if current active montage is unequip, we stop it with blend
	FAnimMontageInstance* Montage = UMontageLibrary::GetAnyActiveMontageInstance(this);
	if (Montage && Montage->Montage && Montage->Montage == RenderActor->UnEquipAnimMontage) {
		Montage->Stop(FAlphaBlend(Montage->GetBlendTime()), false);//TODO: Montage->GetBlend(), 
	}

	//deinit previous render actor
	if (RenderActor) {
		RenderActor->Deinit();
	}
	
	Player->ItemMesh->SetChildActorClass(NextRenderActor);
	NextRenderActor = nullptr;

	//reset lean intensities
	LeanIntensity = FVector2D(1.f);

	//reset pose blend
	PoseBlend = 0.f;
	TargetPoseBlend = 0.f;
	
	if (APlayerInventoryActor* ItemRenderActor = Cast<APlayerInventoryActor>(Player->ItemMesh->GetChildActor())) {
		RenderActor = ItemRenderActor;
		RenderActor->Init(Player);

		//set the player's mesh to use the fov of the item
		if (UViewmodelSkeletalMeshComponent* Mesh = Cast<UViewmodelSkeletalMeshComponent>(Player->GetMesh())) {
			Mesh->ViewmodelData = RenderActor->PlayerViewmodelData;
		}
		
		if (RenderActor->EquipAnimMontage)
			UMontageLibrary::StartMontage(Player->GetMesh(), RenderActor->EquipAnimMontage, CurrentPlayRate);
	}
}

void UPlayerAnimInstance::MontageEnded(UAnimMontage* Montage, const bool bInterrupted) {
	RenderActor->OnMontageComplete(Montage, bInterrupted);
}

void UPlayerAnimInstance::MontageBlendOut(UAnimMontage* Montage, const bool bInterrupted) {
	RenderActor->OnMontageBlendOut(Montage, bInterrupted);
}

void UPlayerAnimInstance::MontageNotifyBegin(const FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload) {
	if (const UAnimMontage* Montage = Cast<UAnimMontage>(BranchingPointPayload.SequenceAsset)) {
		RenderActor->OnMontageNotifyBegin(Montage, NotifyName);
	}
}

void UPlayerAnimInstance::MontageNotifyEnd(const FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload) {
	if (const UAnimMontage* Montage = Cast<UAnimMontage>(BranchingPointPayload.SequenceAsset)) {
		RenderActor->OnMontageNotifyEnd(Montage, NotifyName);
	}
}

UAnimMontage* UPlayerAnimInstance::GetCurrentActiveMontageOfAnimationGroup(const EAnimationGroups AnimationGroup) const {
	const FName GroupName = AnimationGroupToName(AnimationGroup);
	return UMontageLibrary::GetCurrentActiveMontageOfGroup(this, GroupName);
}

bool UPlayerAnimInstance::IsAMontageOfAnimationGroupActive(const EAnimationGroups AnimationGroup, const bool IncludeBlendingOut) const {
	const FName GroupName = AnimationGroupToName(AnimationGroup);
	return UMontageLibrary::IsAMontageOfGroupActive(this, GroupName, IncludeBlendingOut);
}
