#pragma once

#include "Forested/ForestedMinimal.h"
#include "InventoryActor.h"
#include "ItemShop.generated.h"

UCLASS()
class FORESTED_API AItemShop : public AInventoryActor {
	GENERATED_BODY()
	
public:	
	AItemShop():
	AItemShop("ItemShop") {
		PrimaryActorTick.bCanEverTick = false;
	}

	explicit AItemShop(const FString& TypeName, const int Capacity = 0):
	AInventoryActor(TypeName, Capacity) {
		PrimaryActorTick.bCanEverTick = false;
	}

protected:
	virtual void BeginPlay() override;

public:	
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure, Category = "Item Shop")
	bool CanBuyItem(FItemHeap Item, AFPlayer* Player, int Slot) const;

	UFUNCTION(BlueprintCallable, Category = "Item Shop")
	FItemHeap BuyItem(AFPlayer* Player, int Slot);

	UFUNCTION(BlueprintImplementableEvent, Category = "Item Shop")
	void OnBuyItem(FItemHeap Item, AFPlayer* Player, int Slot);

};

UINTERFACE()
class UBuyInterface : public UInterface {
	GENERATED_BODY()
};

class IBuyInterface {
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent)
	void OnItemBought(AItemShop* Shop);
	virtual void OnItemBought_Implementation(AItemShop* Shop) {}

	UFUNCTION(BlueprintNativeEvent)
	void GetShopProperties(int& OutCost, FString& OutCategoryName);
	virtual void GetShopProperties_Implementation(int& OutCost, FString& OutCategoryName) {
		OutCost = 0;
		OutCategoryName = "";
	}

};