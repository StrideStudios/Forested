#include "Tree/ChopStump.h"

AChopStump::AChopStump() {
	
}

bool AChopStump::CanItemBeInserted(const FItemHeap Item, const int Slot) {
	if (!Super::CanItemBeInserted(Item, Slot)) return false;
	if (!Item->GetClass()->ImplementsInterface(UChopStumpInterface::StaticClass())) {
		LOG_EDITOR_WARNING("Item %s could not be added to chop stump; It does not implement the chop stump interface", *Item->GetItemName().ToString());
		return false;
	}
	return true;
}

