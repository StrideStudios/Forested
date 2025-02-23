#pragma once

#include "Forested/ForestedMinimal.h"
#include "GameFramework/Actor.h"
#include "Inventory.h"
#include "Interfaces/SelectableInterface.h"
#include "InventoryActor.generated.h"

UCLASS(meta = (PrioritizeCategories = "Inventory"))
class FORESTED_API AInventoryActor : public AActor, public IInventory {
	GENERATED_BODY()
	
public:
	
	AInventoryActor(): AInventoryActor("Inventory") {
	}

	AInventoryActor(const FString& TypeName, const int Capacity = 0):
	TypeName(TypeName),
	Capacity(Capacity) {
	}

	virtual TMap<int32, FItemHeap>& GetItems() override { return Items; }
	
	virtual const TMap<int32, FItemHeap>& GetItems() const override { return Items; }
	
	virtual FString GetTypeName_Implementation() const override { return TypeName; }

	virtual int GetCapacity() const override { return Capacity; }

protected:
	virtual void BeginPlay() override;
	
private:

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	FString TypeName;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	int Capacity;

	UPROPERTY(SaveGame)
	TMap<int32, FItemHeap> Items;
};

UCLASS()
class FORESTED_API ASelectableInventoryActor : public AInventoryActor, public ISelectableInterface {
	GENERATED_BODY()
	
public:
	
	ASelectableInventoryActor(): ASelectableInventoryActor("Inventory") {
	}

	ASelectableInventoryActor(const FString& TypeName, const int Capacity = 0):
	AInventoryActor(TypeName, Capacity) {
	}

};

UCLASS()
class FORESTED_API AOneSelectableInventoryActor : public ASelectableInventoryActor {
	GENERATED_BODY()
	
public:
	
	AOneSelectableInventoryActor(): AOneSelectableInventoryActor("Inventory") {
	}

	AOneSelectableInventoryActor(const FString& TypeName):
	ASelectableInventoryActor(TypeName, 1) {
	}

	virtual bool Selected_Implementation(AFPlayer* Player, const FHitResult& HitResult, const float HeldTime) override;

private:

	//this determines if it takes a single item from the player rather than an entire item heap
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	bool TakeSingleItem = true;

	//this determines if it gives a single item to the player rather than an entire item heap
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	bool GiveSingleItem = true;
};