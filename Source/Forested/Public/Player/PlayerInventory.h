#pragma once

#include "Forested/ForestedMinimal.h"
#include "Items/InventoryComponent.h"
#include "Animation/AnimInstance.h"
#include "Forested/ForestedGameMode.h"
#include "PlayerInventory.generated.h"

struct FAnimMontageInstance;
class UBlendSpace1D;
class UObjectSaveGame;
class UCurveFloat;
class APlayerInventoryActor;
class ISelectableInterface;
struct FObjectData;

const FName Name_PrimarySlot = "";

UENUM(BlueprintType)
enum class EAnimationGroups : uint8 {
	PrimaryGroup UMETA(DisplayName = "Primary Group"),
	SecondaryGroup UMETA(DisplayName = "Secondary Group")
};

FORCEINLINE FName AnimationGroupToName(const EAnimationGroups AnimationGroups) {
	switch (AnimationGroups) {
	case EAnimationGroups::PrimaryGroup:
		return "PrimaryGroup";
	case EAnimationGroups::SecondaryGroup:
		return "SecondaryGroup";
	}
	return "INVALID";
}

UENUM(BlueprintType)
enum class EMovementType : uint8 {
	Movement UMETA(DisplayName = "Movement"),
	Equipping UMETA(DisplayName = "Equipping"),
	EndEquipping UMETA(DisplayName = "End Equipping"),
	LeftInteract UMETA(DisplayName = "Left Interact"),
	RightLeftInteract UMETA(DisplayName = "Right Left Interact"),
	RightInteract UMETA(DisplayName = "Right Interact"),
	EndRightInteract UMETA(DisplayName = "End Right Interact"),
	ButtonInteract UMETA(DisplayName = "Button Interact")
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent, PrioritizeCategories = "Inventory") )
class FORESTED_API UPlayerInventory : public UInventoryComponent, public IInitInterface {
	GENERATED_BODY()

public:
	
	UPlayerInventory();

protected:

	virtual void BeginPlay() override;

public:	
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player Inventory")
	FORCEINLINE bool IsSelectedSlotValid() const { return SelectedSlot >= 0 && SelectedSlot < GetCapacity(); }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player Inventory")
	FORCEINLINE int GetSelectedSlot() const { return SelectedSlot; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player Inventory")
	FORCEINLINE bool HasSelectedItem() const {
		if (SelectedSlot < 0) return false;
		return HasItem(SelectedSlot);
	}

	//used to remove item from hand (and all anims will not play)
	UFUNCTION(BlueprintCallable, Category = "Player Inventory")
	bool ClearSelectedItem();

	//resets selected item back to previous value if cleared
	UFUNCTION(BlueprintCallable, Category = "Player Inventory")
	bool ResetSelectedItem();
	
	//get an item from the inventory
	FORCEINLINE bool GetSelectedItem(FItemHeap& OutSelectedItem) const {
		if (!IsSelectedSlotValid()) return false;
		return GetItem(SelectedSlot, OutSelectedItem);
	}
	
	void SetSelectedSlot(const int Slot) {
		if (!IsSelectedSlotValid()) return;
		SetSelectedSlot_Internal(Slot);
		RegisterItem();
	}
	
	void ChangeItem(int Change);

	void LeftInteract() const;
	
	void EndLeftInteract() const;

	void RightInteract() const;

	void EndRightInteract() const;

	void ButtonInteract() const;

private:

	UFUNCTION(BlueprintCallable, BlueprintPure, DisplayName = "Get Selected Item", Category = "Player Inventory")
	FORCEINLINE bool ReceiveGetSelectedItem(FItemHeap& OutSelectedItem) const {
		return GetSelectedItem(OutSelectedItem);
	}

	void SetSelectedSlot_Internal(int Slot);

	void RegisterItem() const;

	/*
	 * Inventory Overrides
	 */

	virtual bool CanItemBeInserted(FItemHeap Item, const int Slot) override;

	virtual bool CanItemBeRemoved(const int Slot) override;
	
	virtual void OnInsertItem(FItemHeap Item, int Slot) override;

	virtual void OnRemoveItem(FItemHeap Item, int Slot) override;

	virtual void OnAppendItems(const TMap<int32, FItemHeap>& AppendedItems) override;

	virtual void Init_Implementation(UObject* WorldContextObject) override;
	
	virtual void LoadDefaults_Implementation(UObject* WorldContextObject) override;

	UPROPERTY(SaveGame)
	int SelectedSlot = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Defaults")
	TSubclassOf<APlayerInventoryActor> DefaultInventoryRenderActor;
	
	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Defaults")
	TArray<FItemHeap> DefaultItems;

	UPROPERTY()
	AFPlayer* Player = nullptr;

};

UCLASS(meta = (PrioritizeCategories = "Animation"))
class FORESTED_API UPlayerAnimInstance : public UAnimInstance, public IInitInterface {
	GENERATED_BODY()

public:

	//TODO: better naming except im lazy as hell
	
	FAnimMontageInstance* GetAnyActiveMontageInstance(bool IncludeBlendingOut = false) const;

	FAnimMontageInstance* GetActiveMontageInstance(const UAnimMontage* Montage, const bool IncludeBlendingOut = false) const {
		FAnimMontageInstance* MontageInstance = GetActiveInstanceForMontage(Montage);
		if (MontageInstance && IsMontageInstanceActive(MontageInstance, IncludeBlendingOut))
			return MontageInstance;
		return nullptr;
	}

	FAnimMontageInstance* GetMontageInstance(const UAnimMontage* Montage = nullptr, const bool IncludeBlendingOut = false) const {
		if (!Montage) return GetAnyActiveMontageInstance(IncludeBlendingOut);
		return GetActiveMontageInstance(Montage, IncludeBlendingOut);
	}

	/*
	 * blueprint accessibility functions
	 */

	/** Get a current Active Montage in this AnimInstance of a certain group. 
		Note that there might be multiple Active at the same time. This will only return the first active one it finds. **/
	UFUNCTION(BlueprintPure, Category = "Montage", meta = (NotBlueprintThreadSafe))
	UAnimMontage* GetCurrentActiveMontageOfGroup(EAnimationGroups AnimationGroup) const;

	UAnimMontage* GetCurrentActiveMontageOfGroup(FName GroupName) const;

	//returns true if any montage is active
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (AdvancedDisplay), Category = "Inventory Render Actor|Montage")
	bool IsAMontageOfGroupActive(EAnimationGroups AnimationGroup, bool IncludeBlendingOut = false) const;
	
	bool IsAMontageOfGroupActive(FName GroupName, bool IncludeBlendingOut = false) const;
	
	//returns true if any montage is active
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (AdvancedDisplay), Category = "Inventory Render Actor|Montage")
	bool IsAMontageActive(bool IncludeBlendingOut = false) const;

	//returns true if any montage is active and playing
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (AdvancedDisplay = 1), Category = "Inventory Render Actor|Montage")
	bool IsAMontagePlaying(bool IncludeBlendingOut = false) const;

	//returns true if Montage is active
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (AdvancedDisplay = 1), Category = "Inventory Render Actor|Montage")
	bool IsMontageActive(const UAnimMontage* Montage, const bool IncludeBlendingOut = false) const {
		return IsMontageInstanceActive(GetActiveInstanceForMontage(Montage), IncludeBlendingOut);
	}

	//returns true if Montage is active and playing
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (AdvancedDisplay = 1), Category = "Inventory Render Actor|Montage")
	bool IsMontagePlaying(const UAnimMontage* Montage, const bool IncludeBlendingOut = false) const {
		return IsMontageInstancePlaying(GetActiveInstanceForMontage(Montage), IncludeBlendingOut);
	}

	static bool IsMontageInstanceActive(const FAnimMontageInstance* Montage, const bool IncludeBlendingOut = false) {
		return Montage && Montage->Montage && IsMontageActive_Internal(Montage, IncludeBlendingOut) && Montage->IsActive();
	}

	static bool IsMontageInstancePlaying(const FAnimMontageInstance* Montage, const bool IncludeBlendingOut = false) {
		return Montage && Montage->Montage && IsMontageActive_Internal(Montage, IncludeBlendingOut) && Montage->IsPlaying() && Montage->GetPosition() != Montage->GetPreviousPosition();
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player Animation Instance")
	FORCEINLINE bool CanSwitchItems() const;
	
	FORCEINLINE APlayerInventoryActor* GetInventoryRenderActor() const { return RenderActor; }

	virtual void Init_Implementation(UObject* WorldContextObject) override;
	
	virtual void NativeBeginPlay() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	virtual bool HandleNotify(const FAnimNotifyEvent& AnimNotifyEvent) override;
	
	void SetNewInventoryRenderActor(const TSubclassOf<APlayerInventoryActor>& InRenderActor);

	void LoadAnimationData();

	UFUNCTION()
	void MontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void MontageBlendOut(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void MontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);
	
	UFUNCTION()
	void MontageNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

	UPROPERTY(BlueprintReadWrite, Category = "Animation|Player")
	float TargetPoseBlend = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Animation|Player")
	FVector2D LeanIntensity = FVector2D(1.f);
	
protected:

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Player")
	float PoseBlend = 0.f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Player")
	FVector Velocity = FVector(0.f);

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Player")
	float ForwardMovement = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Player")
	float RightMovement = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Player")
	float ForwardVelocity = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Player")
	float RightVelocity = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Player")
	FRotator CameraRotation = FRotator();
	
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Player")
	float UnitVelocity = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Player")
	float UnitVelocity2D = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Player")
	float VerticalVelocity = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Player")
	bool IsMoving = false;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Player")
	bool IsFalling = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Player")
	bool IsSwimming = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	APlayerInventoryActor* RenderActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	AFPlayer* Player = nullptr;

	UPROPERTY()
	TSubclassOf<APlayerInventoryActor> NextRenderActor = nullptr;

private:

	static bool IsMontageActive_Internal(const FAnimMontageInstance* MontageInstance, const bool IncludeBlendingOut);
	
};

UINTERFACE()
class UPlayerInventoryInterface : public UInterface {
	GENERATED_BODY()
};

class IPlayerInventoryInterface {
	GENERATED_BODY()

public:

	/**
	 * Event that fires when an item is added to the player's inventory
	 * @param Inventory The inventory it was added to
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Player Inventory Interface")
	void OnAddedToPlayerInventory(UPlayerInventory* Inventory);
	virtual void OnAddedToPlayerInventory_Implementation(UPlayerInventory* Inventory) {}

	/**
	 * Get Properties for the item added into the inventory
	 * @param InventoryRenderClass the class used to render the item in the player's hand
	 * @param StaticMesh an override mesh (not used unless for generics i think)
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Player Inventory Interface")
	void GetPlayerInventoryProperties(TSoftClassPtr<APlayerInventoryActor>& InventoryRenderClass, TSoftObjectPtr<UStaticMesh>& StaticMesh) const;
	virtual void GetPlayerInventoryProperties_Implementation(TSoftClassPtr<APlayerInventoryActor>& InventoryRenderClass, TSoftObjectPtr<UStaticMesh>& StaticMesh) const {
		InventoryRenderClass = nullptr;
		StaticMesh = nullptr;
	}

};