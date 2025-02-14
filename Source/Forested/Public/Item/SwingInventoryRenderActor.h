#pragma once

#include "Forested/ForestedMinimal.h"
#include "AlphaBlend.h"
#include "DamageableInterface.h"
#include "PlayerInventoryActor.h"
#include "SwingInventoryRenderActor.generated.h"

class UNiagaraComponent;
class UCameraShakeBase;
class UNiagaraSystem;
class UCurveVector;

UCLASS(BlueprintType, meta = (PrioritizeCategories = "Damage"))
class FORESTED_API ASwingInventoryRenderActor : public APlayerInventoryActor {
	GENERATED_BODY()

public:
	
	ASwingInventoryRenderActor();

	virtual void InventoryTick(float DeltaTime) override;
	
	virtual void OnMontageNotifyBegin(const UAnimMontage* Montage, const FName Notify) override;
	
	virtual void OnMontageNotifyEnd(const UAnimMontage* Montage, const FName Notify) override;

	virtual void OnMontageBlendOut(const UAnimMontage* Montage, const bool bInterrupted) override;

	virtual bool CanMontagePlay_Implementation(const UAnimMontage* Montage, float PlayRate, float StartingPosition) const override;
	
	UFUNCTION(BlueprintCallable, Category = "Swing Inventory Render Actor")
	bool Swing(UAnimMontage* Montage, const FAlphaBlend& InHitBlend, float InHitDelay = 0.3f, float PlayRate = 1.f, float StartingPosition = 0.f);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Swing Inventory Render Actor")
	FORCEINLINE bool IsSwinging() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Swing Inventory Render Actor")
	FORCEINLINE bool CanSwing() const { return Delay <= 0.f && !IsSwinging(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Swing Inventory Render Actor")
	FORCEINLINE bool HasHit() const { return bHit; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Swing Inventory Render Actor")
	FORCEINLINE bool CanHit() const { return bCanHit; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	USceneComponent* StartTraceMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	USceneComponent* StartTrace;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	USceneComponent* EndTrace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	UNiagaraComponent* WeaponSwing;

private:

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	EDamageType DamageType = EDamageType::Axe;
	
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float Damage = 15.f;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	UNiagaraSystem* HitNiagaraSystem = nullptr;
	
	float HitDelay = 0.f;

	FAlphaBlend HitBlend = FAlphaBlend(0.f);
	
	UPROPERTY()
	UAnimMontage* SwingMontage;
	
	FTransform PTraceTransform;

	bool bCanHit = false;
	
	bool bHit = false;

	float Delay = 0.f;
	
};
