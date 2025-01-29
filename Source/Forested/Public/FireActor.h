#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SelectableInterface.h"
#include "FireActor.generated.h"

UCLASS()
class AFireActor : public AActor, public ISelectableInterface {
	GENERATED_BODY()

public:

	AFireActor();

	virtual bool Selected_Implementation(AFPlayer* Player, const FHitResult& HitResult, const float HeldTime) override;

	UPROPERTY(BlueprintReadOnly, Category = "Fire Actor")
	bool bIsBurning = false;

	UPROPERTY(BlueprintReadOnly, Category = "Fire Actor")
	float TotalBurnTime = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire Actor")
	float MaxBurnTime = 100.f;
};

UINTERFACE()
class UFireFuelInterface : public UInterface {
	GENERATED_BODY()
};

class IFireFuelInterface {
	GENERATED_BODY()

public:

	/**
	 * Event that fires when an item is put in the fire
	 * @param FireActor The fire it was put in
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Fire Actor Interface")
	void OnAddedToFire(AFireActor* FireActor);
	virtual void OnAddedToFire_Implementation(AFireActor* FireActor) {}

	/**
	 * Get Properties for the fire actor
	 * @param BurnTime The time the item will burn for (in seconds)
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Fire Actor Interface")
	void GetFireFuelProperties(float& BurnTime) const;
	virtual void GetFireFuelProperties_Implementation(float& BurnTime) const {
		BurnTime = 120.f;
	}

};

UINTERFACE()
class ULightFireInterface : public UInterface {
	GENERATED_BODY()
};

class ILightFireInterface {
	GENERATED_BODY()

public:
	
	/**
	 * Get Properties for the fire actor when trying to light it
	 * @param StartChance The chance for the fire to start 0-100
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Fire Actor Interface")
	void GetLightFireProperties(float& StartChance) const;
	virtual void GetFireActorProperties_Implementation(float& StartChance) const {
		StartChance = 60.f;
	}

};