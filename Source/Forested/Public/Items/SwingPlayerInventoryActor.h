#pragma once

#include "Forested/ForestedMinimal.h"
#include "AlphaBlend.h"
#include "Player/PlayerInventoryActor.h"
#include "SwingPlayerInventoryActor.generated.h"

enum class EDamageType : uint8;
class UNiagaraComponent;
class UCameraShakeBase;
class UNiagaraSystem;
class UCurveVector;

UCLASS(BlueprintType, meta = (PrioritizeCategories = "Damage"))
class FORESTED_API ASwingPlayerInventoryActor : public APlayerInventoryActor {
	GENERATED_BODY()

public:
	
	ASwingPlayerInventoryActor();

	virtual void InventoryTick(float DeltaTime) override;
	
	virtual void OnMontageNotifyBegin(const UAnimMontage* Montage, const FName Notify) override;
	
	virtual void OnMontageNotifyEnd(const UAnimMontage* Montage, const FName Notify) override;

	virtual void OnMontageBlendOut(const UAnimMontage* Montage, const bool bInterrupted) override;
	
	UFUNCTION(BlueprintCallable, Category = "Swing Player Inventory Actor")
	bool Swing(UAnimMontage* Montage, const FAlphaBlend& InHitBlend, float InHitDelay = 0.3f, float PlayRate = 1.f, float StartingPosition = 0.f);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Swing Player Inventory Actor")
	FORCEINLINE bool IsSwinging() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Swing Player Inventory Actor")
	FORCEINLINE bool CanSwing() const { return Delay <= 0.f && !IsSwinging(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Swing Player Inventory Actor")
	FORCEINLINE bool HasHit() const { return bHit; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Swing Player Inventory Actor")
	FORCEINLINE bool CanHit() const { return bCanHit; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	USceneComponent* StartTraceMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	USceneComponent* StartTrace;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	USceneComponent* EndTrace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	UNiagaraComponent* WeaponSwing;

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	UNiagaraSystem* HitNiagaraSystem = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float Damage = 15.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	EDamageType DamageType;

private:
	
	float HitDelay = 0.f;

	FAlphaBlend HitBlend = FAlphaBlend(0.f);
	
	UPROPERTY()
	UAnimMontage* SwingMontage;
	
	FTransform PTraceTransform;

	bool bCanHit = false;
	
	bool bHit = false;

	float Delay = 0.f;
	
};
