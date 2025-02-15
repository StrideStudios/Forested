#include "Item/InventoryActor.h"
#include "FPlayer.h"
#include "Item/PlayerInventory.h"

void AInventoryActor::BeginPlay() {
	Super::BeginPlay();
	//reserve memory for items to reduce lag from allocating memory
	Items.Reserve(Capacity);
}

bool AOneSelectableInventoryActor::Selected_Implementation(AFPlayer* Player, const FHitResult& HitResult, const float HeldTime) {
	//take an item from the player inventory
	if (TakeSingleItem ?
		SwapItemFrom(Player->PlayerInventory, Player->PlayerInventory->GetSelectedSlot(), 0) :
		SwapHeapFrom(Player->PlayerInventory, Player->PlayerInventory->GetSelectedSlot(), 0))
		return true;
	//give an item to the player inventory
	if (GiveSingleItem ?
		Player->PlayerInventory->SwapItemFrom(this, 0, Player->PlayerInventory->GetSelectedSlot()) :
		Player->PlayerInventory->SwapHeapFrom(this, 0, Player->PlayerInventory->GetSelectedSlot()))
		return true;
	return false;
}