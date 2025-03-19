#pragma once

#include "Forested/ForestedMinimal.h"
#include "Player/PlayerInventoryActor.h"
#include "Widget/PlayerWidget.h"
#include "ShootPlayerInventoryActor.generated.h"

class UShootItem;
class UShootWidget;
class UNiagaraSystem;

UCLASS(BlueprintType, meta = (PrioritizeCategories = "Damage"))
class FORESTED_API AShootPlayerInventoryActor : public APlayerInventoryActor {
	GENERATED_BODY()

public:
	
	AShootPlayerInventoryActor();

	virtual void Init(AFPlayer* InPlayer) override;

	virtual void Deinit() override;

	virtual void OnMontageBlendOut(const UAnimMontage* Montage, const bool bInterrupted) override;

	/**
	 * Getters 
	 */

	UFUNCTION(BlueprintCallable, Category = "Shoot Player Inventory Actor")
	FORCEINLINE UNiagaraSystem* GetHitNiagaraSystem() const { return HitNiagaraSystem; }

	UFUNCTION(BlueprintCallable, Category = "Shoot Player Inventory Actor")
	FORCEINLINE TSoftClassPtr<UShootWidget> GetWidgetClass() const { return WidgetClass; }

	UFUNCTION(BlueprintCallable, Category = "Shoot Player Inventory Actor")
	FORCEINLINE UShootWidget* GetShootWidget() const { return ShootWidget; }

	UFUNCTION(BlueprintCallable, Category = "Shoot Player Inventory Actor")
	FORCEINLINE int GetAmountOfProjectiles() const { return AmountOfProjectiles; }

	UFUNCTION(BlueprintCallable, Category = "Shoot Player Inventory Actor")
	FORCEINLINE float GetRange() const { return Range; }

	UFUNCTION(BlueprintCallable, Category = "Shoot Player Inventory Actor")
	FORCEINLINE FVector2D GetMaxDamage() const { return MaxDamage; }

	UFUNCTION(BlueprintCallable, Category = "Shoot Player Inventory Actor")
	FORCEINLINE FVector2D GetSpread() const { return Spread; }

	UFUNCTION(BlueprintCallable, Category = "Shoot Player Inventory Actor")
	FORCEINLINE FVector2D GetShootPitch() const { return ShootPitch; }

	UFUNCTION(BlueprintCallable, Category = "Shoot Player Inventory Actor")
	FORCEINLINE FVector2D GetLeanIntensity() const { return LeanIntensity; }
	
	UFUNCTION(BlueprintCallable, Category = "Shoot Player Inventory Actor")
	FORCEINLINE int GetMaxBullets() const { return MaxBullets; }

	UFUNCTION(BlueprintCallable, Category = "Shoot Player Inventory Actor")
	FORCEINLINE bool IsShooting() const { return ShootMontage != nullptr; }

	UFUNCTION(BlueprintCallable, Category = "Shoot Player Inventory Actor")
	FORCEINLINE bool IsReloading() const { return ReloadMontage != nullptr; }

	/**
	 * Blueprint Functions
	 */
	
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = 2), Category = "Shoot Player Inventory Actor")
	bool TraceShot(FHitResult& OutHit, FVector ShootLocation, float Rotation, float InSpread);
	
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = 2), Category = "Shoot Player Inventory Actor")
	bool Shoot(UAnimMontage* Montage, FViewmodelVector ShootLocation, float PlayRate = 1.f, float StartingPosition = 0.f, bool Aimed = false);
	
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = 1), Category = "Shoot Player Inventory Actor")
	bool Reload(UAnimMontage* Montage, float PlayRate = 1.f, float StartingPosition = 0.f, bool FullReload = true, int BulletsToAdd = 0);
	
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = 1), Category = "Shoot Player Inventory Actor")
	bool Aim(UAnimMontage* Montage, float PlayRate = 1.f, float StartingPosition = 0.f);

	UFUNCTION(BlueprintCallable, Category = "Shoot Player Inventory Actor")
	bool UnAim(UAnimMontage* Montage);

	UFUNCTION(BlueprintCallable, Category = "Shoot Player Inventory Actor")
	void TransformWidgetToShootPoint(float DeltaSeconds, FVector ShootLocation);
	
	/**
	 * Blueprint Implementable Functions
	 */
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Shoot Player Inventory Actor")
	USceneComponent* GetMesh();

	UFUNCTION(BlueprintImplementableEvent, Category = "Shoot Player Inventory Actor")
	FViewmodelData GetViewmodelData();

	UFUNCTION(BlueprintImplementableEvent, Category = "Shoot Player Inventory Actor")
	void EndShot();

	UFUNCTION(BlueprintImplementableEvent, Category = "Shoot Player Inventory Actor")
	void EndReload();

protected:

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (BlueprintProtected), Category = "Inventory Render Actor")
	bool GetShootItem(UShootItem*& OutItem) const;
	
private:

	/*
	 * Class Defaults
	 */
	
	UPROPERTY(EditDefaultsOnly, Category = "Shooting")
	UNiagaraSystem* HitNiagaraSystem = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Shooting")
	TSoftClassPtr<UShootWidget> WidgetClass = nullptr;
	
	UPROPERTY(EditDefaultsOnly, Category = "Shooting")
	int AmountOfProjectiles = 1;
	
	UPROPERTY(EditDefaultsOnly, Category = "Shooting")
	float Range = 10000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Shooting")
	FVector2D MaxDamage = FVector2D(15.f, 25.f);

	UPROPERTY(EditDefaultsOnly, Category = "Shooting")
	FVector2D Spread = FVector2D(0.025f, 0.01f);

	UPROPERTY(EditDefaultsOnly, Category = "Shooting")
	FVector2D ShootPitch = FVector2D(0.25f, 0.1f);

	UPROPERTY(EditDefaultsOnly, Category = "Shooting")
	FVector2D LeanIntensity = FVector2D(0.25f, 0.3f);
	
	UPROPERTY(EditDefaultsOnly, Category = "Reloading")
	int MaxBullets = 10;

	UPROPERTY()
	UShootWidget* ShootWidget;
	
	UPROPERTY()
	UAnimMontage* ShootMontage;

	UPROPERTY()
	UAnimMontage* ReloadMontage;

	FVector2D ShootWidgetTranslation;

};

UCLASS(Blueprintable)
class FORESTED_API UShootWidget : public UPlayerWidget {
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent, Category = "Shoot Widget")
	void OnAim(AShootPlayerInventoryActor* ShootPlayerInventoryActor);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Shoot Widget")
	void OnUnaim(AShootPlayerInventoryActor* ShootPlayerInventoryActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Shoot Widget")
	void TransformWidgetToShootPoint(FVector2D Translation);
	
};