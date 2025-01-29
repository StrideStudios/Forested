#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory.h"
#include "ObjectSaveGame.h"
#include "InventoryComponent.generated.h"

UCLASS(meta = (PrioritizeCategories = "Inventory"))
class FORESTED_API UInventoryComponent : public UActorComponent, public IInventory {
	GENERATED_BODY()
	
public:
	
	UInventoryComponent(): UInventoryComponent("Inventory Component") {
	}

	UInventoryComponent(const FString& TypeName, const int Capacity = 0):
	TypeName(TypeName),
	Capacity(Capacity) {
	}

	virtual TMap<int32, FItemHeap>& GetItems() override { return Items; }
	
	virtual const TMap<int32, FItemHeap>& GetItems() const override { return Items; }
	
	virtual int GetCapacity() const override { return Capacity; }

	virtual FString GetTypeName_Implementation() const override { return TypeName; }
	
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