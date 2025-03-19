#pragma once

#include "Forested/ForestedMinimal.h"
#include "ViewmodelMeshes.h"
#include "GameFramework/Actor.h"
#include "PlayerInventoryActor.generated.h"

class UItem;
class UBlendSpace1D;
struct FItemHeap;
struct FAlphaBlend;
class UViewmodelStaticMeshComponent;
class UPlayerAnimInstance;

/*
 *	Used to define logic for rendering an item in inventory
 */
UCLASS(meta = (PrioritizeCategories ="Animation"))
class FORESTED_API APlayerInventoryActor : public AActor {
	GENERATED_BODY()

public:	
	APlayerInventoryActor();

	virtual void Init(AFPlayer* InPlayer);
	
	virtual void Deinit();
	
	virtual void InventoryTick(const float DeltaTime) {
	    ReceiveInventoryTick(DeltaTime);
	}

	UFUNCTION(BlueprintNativeEvent, Category = "Inventory Render Actor")
	bool CanSwitchItems() const;
	virtual bool CanSwitchItems_Implementation() const { return true; }
	
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Inventory Tick", Category = "Inventory Render Actor")
    void ReceiveInventoryTick(float DeltaTime);
	
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Init", Category = "Inventory Render Actor")
	void ReceiveInit();

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Deinit", Category = "Inventory Render Actor")
	void ReceiveDeinit();
	
	virtual void OnMontageComplete(const UAnimMontage* Montage, const bool bInterrupted) {
		ReceiveOnMontageComplete(Montage , bInterrupted);
	}

	virtual void OnMontageBlendOut(const UAnimMontage* Montage, const bool bInterrupted) {
		ReceiveOnMontageBlendOut(Montage, bInterrupted);
	}

	virtual void OnMontageNotifyBegin(const UAnimMontage* Montage, const FName Notify);

	virtual void OnMontageNotifyEnd(const UAnimMontage* Montage, const FName Notify) {
		ReceiveOnMontageNotifyEnd(Montage, Notify);
	}
	
	virtual void OnLeftInteract();
	
	virtual void OnLeftEndInteract();

	virtual void OnRightInteract();

	virtual void OnRightEndInteract();

	virtual void OnButtonInteract();
	
	virtual void OnReload();
	
	virtual void OnEndReload();

	virtual void OnManualBlendOutMontageComplete(const UAnimMontage* Montage) {
		ReceiveOnManualBlendOutMontageComplete(Montage);
	}

	//viewmodel data to use for player arms mesh
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Viewmodel")
	FViewmodelData PlayerViewmodelData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UViewmodelStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBlendSpace1D* MovementBlendSpace = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* EquipAnimMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* UnEquipAnimMontage = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimSequence* JumpAnimSequence = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimSequence* FallingAnimSequence = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimSequence* LandAnimSequence = nullptr;
	
protected:

	void SetupRootAttachment() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintProtected), Category = "Inventory Render Actor")
	FORCEINLINE AFPlayer* GetPlayer() const { return Player; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintProtected), Category = "Inventory Render Actor")
	UPlayerAnimInstance* GetPlayerAnimInstance() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintProtected), Category = "Inventory Render Actor")
	bool HasItem() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintProtected), Category = "Inventory Render Actor")
	bool GetItem(FItemHeap& OutItem) const;
	
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Montage Complete", Category = "Inventory Render Actor|Montage")
	void ReceiveOnMontageComplete(const UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Montage Blend Out", Category = "Inventory Render Actor|Montage")
	void ReceiveOnMontageBlendOut(const UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Montage Notify Begin", Category = "Inventory Render Actor|Montage")
	void ReceiveOnMontageNotifyBegin(const UAnimMontage* Montage, FName Notify);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Montage Notify End", Category = "Inventory Render Actor|Montage")
	void ReceiveOnMontageNotifyEnd(const UAnimMontage* Montage, FName Notify);
	
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Manual Blend Out Montage Complete", Category = "Inventory Render Actor|Montage")
	void ReceiveOnManualBlendOutMontageComplete(const UAnimMontage* Montage);
	
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Left Interact", Category = "Inventory Render Actor|Player")
	void ReceiveOnLeftInteract(bool IsMontagePlaying);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Left End Interact", Category = "Inventory Render Actor|Player")
	void ReceiveOnLeftEndInteract(bool IsMontagePlaying);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Right Interact", Category = "Inventory Render Actor|Player")
	void ReceiveOnRightInteract(bool IsMontagePlaying);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Right End Interact", Category = "Inventory Render Actor|Player")
	void ReceiveOnRightEndInteract(bool IsMontagePlaying);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Button Interact", Category = "Inventory Render Actor|Player")
	void ReceiveOnButtonInteract(bool IsMontagePlaying);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Reload", Category = "Inventory Render Actor|Player")
	void ReceiveOnReload(bool IsMontagePlaying);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On End Reload", Category = "Inventory Render Actor|Player")
	void ReceiveOnEndReload(bool IsMontagePlaying);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Render")
	FName AttachSocket = "Socket_R_Default";

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bAnimationsLoaded = false;

private:

	UPROPERTY()
	AFPlayer* Player;
	
};