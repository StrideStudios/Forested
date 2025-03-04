#pragma once

#include "Forested/ForestedMinimal.h"
#include "Player/PlayerInventoryActor.h"
#include "ShootPlayerInventoryActor.generated.h"

class UNiagaraSystem;

UCLASS(BlueprintType, meta = (PrioritizeCategories = "Damage"))
class FORESTED_API AShootPlayerInventoryActor : public APlayerInventoryActor {
	GENERATED_BODY()

public:
	
	AShootPlayerInventoryActor();

	UFUNCTION(BlueprintCallable, Category = "Shoot Player Inventory Actor")
	bool Shoot(UAnimMontage* Montage, FViewmodelVector ShootLocation, float PlayRate = 1.f, float StartingPosition = 0.f, bool Aimed = false);

	UFUNCTION(BlueprintImplementableEvent, Category = "Shoot Player Inventory Actor")
	USceneComponent* GetMesh();
	
protected:
	
	UFUNCTION(BlueprintCallable, Category = "Shoot Player Inventory Actor")
	bool TraceShot(FHitResult& OutHit, FVector ShootLocation, float Rotation, float InSpread);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shooting")
	UNiagaraSystem* HitNiagaraSystem = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shooting")
	int AmountOfProjectiles = 1;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shooting")
	float Range = 10000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shooting")
	FVector2D MaxDamage = FVector2D(15.f, 25.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shooting")
	FVector2D Spread = FVector2D(0.025f, 0.01f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shooting")
	FVector2D ShootPitch = FVector2D(0.25f, 0.1f);
};
