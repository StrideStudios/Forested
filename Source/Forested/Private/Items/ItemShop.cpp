#include "Items/ItemShop.h"
#include "Player/FPlayer.h"
#include "Items/Item.h"

void AItemShop::BeginPlay() {
	Super::BeginPlay();
	
}

FItemHeap AItemShop::BuyItem(AFPlayer* Player, const int Slot) {
	FItemHeap Item; 
	if (GetItem(Slot, Item)) {
		if (Item->GetClass()->ImplementsInterface(UBuyInterface::StaticClass())) {
			int Cost;
			FString Category;
			IBuyInterface::Execute_GetShopProperties(Item, Cost, Category);
			if (Player->GetMoney() < Cost || !CanBuyItem(Item, Player, Slot))
				return {};
			Player->AddMoney(-Cost);
			OnBuyItem(Item, Player, Slot);
			IBuyInterface::Execute_OnItemBought(Item, this);
			return Item;
		}
	}
	return {};
}
