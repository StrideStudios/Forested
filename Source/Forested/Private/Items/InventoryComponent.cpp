#include "Items/InventoryComponent.h"

void UInventoryComponent::BeginPlay() {
	Super::BeginPlay();
	//reserve memory for items to reduce lag from allocating memory
	Items.Reserve(Capacity);
}
