#pragma once

#include "ViewmodelMeshes.h"
#include "Forested/ForestedMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerInventoryActor.generated.h"

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

	virtual void Init();
	
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
	
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "Inventory Render Actor|Montage")
	bool StartMontage(UAnimMontage* MontageToPlay, float PlayRate = 1.f, float StartingPosition = 0.f) const; 

	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "Inventory Render Actor|Montage")
	bool StartStackedMontage(UAnimMontage* MontageToPlay, float PlayRate = 1.f, float StartingPosition = 0.f) const; 

	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "Inventory Render Actor|Montage")
	bool PauseMontage(const UAnimMontage* Montage = nullptr) const; 

	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "Inventory Render Actor|Montage")
	bool ResumeMontage(const UAnimMontage* Montage = nullptr) const; 

	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "Inventory Render Actor|Montage")
	bool StopMontage(float BlendOutTime, const UAnimMontage* Montage = nullptr) const; 

	bool StopMontage(const FAlphaBlend& Blend, const UAnimMontage* Montage = nullptr) const; 
	
	virtual void OnMontageComplete(const UAnimMontage* Montage, const bool bInterrupted) {
		ReceiveOnMontageComplete(Montage , bInterrupted);
	}

	virtual void OnMontageBlendOut(const UAnimMontage* Montage, const bool bInterrupted) {
		ReceiveOnMontageBlendOut(Montage, bInterrupted);
	}

	virtual void OnMontageNotifyBegin(const UAnimMontage* Montage, const FName Notify) {
		ReceiveOnMontageNotifyBegin(Montage, Notify);
	}

	virtual void OnMontageNotifyEnd(const UAnimMontage* Montage, const FName Notify) {
		ReceiveOnMontageNotifyEnd(Montage, Notify);
	}
	
	virtual void OnLeftInteract();
	
	virtual void OnLeftEndInteract();

	virtual void OnRightInteract();

	virtual void OnRightEndInteract();

	virtual void OnButtonInteract();

	virtual void OnManualBlendOutMontageComplete(const UAnimMontage* Montage) {
		ReceiveOnManualBlendOutMontageComplete(Montage);
	}
	
	UFUNCTION(BlueprintNativeEvent, Category = "Inventory Render Actor|Montage")
	bool CanMontagePlay(const UAnimMontage* Montage, float PlayRate, float StartingPosition) const;
	virtual bool CanMontagePlay_Implementation(const UAnimMontage* Montage, float PlayRate, float StartingPosition) const { return true; }

	UFUNCTION(BlueprintNativeEvent, Category = "Inventory Render Actor|Montage")
	bool CanMontagePause(const UAnimMontage* Montage) const;
	virtual bool CanMontagePause_Implementation(const UAnimMontage* Montage) const { return true; }

	UFUNCTION(BlueprintNativeEvent, Category = "Inventory Render Actor|Montage")
	bool CanMontageResume(const UAnimMontage* Montage) const;
	virtual bool CanMontageResume_Implementation(const UAnimMontage* Montage) const { return true; }

	UFUNCTION(BlueprintNativeEvent, Category = "Inventory Render Actor|Montage")
	bool CanMontageStop(float BlendOutTime, const UAnimMontage* Montage) const;
	virtual bool CanMontageStop_Implementation(float BlendOutTime,const UAnimMontage* Montage) const { return true; }

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
	static UPlayerAnimInstance* GetPlayerAnimInstance();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintProtected), Category = "Inventory Render Actor")
	static bool HasItem();

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintProtected), Category = "Inventory Render Actor")
	static bool GetItem(FItemHeap& OutItem);
	
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Render")
	FName AttachSocket = "Socket_R_Default";

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bAnimationsLoaded = false;
	
};