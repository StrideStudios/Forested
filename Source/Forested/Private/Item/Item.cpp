#include "Item/Item.h"

UItem* UItem::ConstructItem(const UObject* WorldContextObject, const TSubclassOf<UItem> ItemClass) {
	if (!ItemClass) {
		LOG_WARNING("UItem::ConstructItem no class specified");
		return nullptr;
	}
	if (!WorldContextObject) {
		LOG_WARNING("UItem::ConstructItem world context is null");
		return nullptr;
	}
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) {
		LOG_WARNING("UItem::ConstructItem could not get world from context object");
		return nullptr;
	}
	return NewObject<UItem>(WorldContextObject->GetWorld(), ItemClass);
}

void FItemHeap::SaveItemHeap() {
	ItemData.Empty();
	for (const UItem* Item : Items) {
		ItemData.Push(FObjectData(Item));
	}
}

void FItemHeap::LoadItemHeap(UObject* Outer) {
	Items.Empty();
	for (FObjectData Data : ItemData) {
		Items.Push(Data.LoadNewObject<UItem>(Outer));
	}
	ItemData.Empty();
}
