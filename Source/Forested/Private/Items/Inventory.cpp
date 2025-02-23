#include "Items/Inventory.h"
#include "Items/Item.h"

void IInventoryBase::OnGameSave_Implementation(UObject* WorldContextObject, UObjectSaveGame* SaveGame) {
	for (auto& Item : GetItems())
		Item.Value.SaveItemHeap();
}

void IInventoryBase::OnGameLoad_Implementation(UObject* WorldContextObject, UObjectSaveGame* SaveGame) {
	for (auto& Item : GetItems())
		Item.Value.LoadItemHeap(GetObject()->GetWorld());
}

bool IInventoryBase::InsertItem(const FItemHeap Item, const int Slot) {
	if (ConsolidateItem(Item, Slot))
		return true;
	if (HasItem(Slot) || !CanItemBeInserted(Item, Slot)) return false;
	GetItems().Emplace(Slot, Item);
	Item->OnAddedToInventory(GetObject(), Slot);
	OnInsertItem(Item, Slot);
	return true;
}

UItem* IInventoryBase::RemoveItem(const int Slot) {
	if (!IsValidSlot(Slot) || !HasItem(Slot) || !CanItemBeRemoved(Slot)) return nullptr;
	UItem* Item = GetItems()[Slot].Pop();
	OnRemoveItem(Item, Slot);
	if (GetItems()[Slot].GetCount() <= 0)
		GetItems().Remove(Slot);
	return Item;
}

FItemHeap IInventoryBase::RemoveHeap(const int Slot) {
	if (!IsValidSlot(Slot) || !HasItem(Slot) || !CanItemBeRemoved(Slot)) return {};
	FItemHeap Item;
	GetItems().RemoveAndCopyValue(Slot, Item);
	OnRemoveItem(Item, Slot);
	return Item;
}

int IInventoryBase::AddItem(const FItemHeap Item, const int Slot) {
	const int i = AddItem_Internal(Item, Slot);
	if (i >= 0) {
		Item->OnAddedToInventory(GetObject(), i);
		OnInsertItem(Item, i);
	}
	return i;
}

bool IInventoryBase::AppendItems(const TArray<FItemHeap>& Items, TArray<FItemHeap>& OutItems) {
	TMap<int32, FItemHeap> AddedItems;
	for (int i = 0; i < Items.Num(); ++i) {
		const FItemHeap Item = Items[i];
		const int Slot = AddItem_Internal(Item, i);
		if (Slot >= 0) {
			Item->OnAddedToInventory(GetObject(), Slot);
			AddedItems.Emplace(Slot, Item);
			continue;
		}
		OutItems.Add(Item);
	}
	OnAppendItems(AddedItems);
	return OutItems.Num() <= 0;
}


FItemHeap IInventoryBase::ReplaceItem(const FItemHeap Item, const int Slot) {
	if (!Item || !IsValidSlot(Slot) || !CanItemBeRemoved(Slot)) return {};
	FItemHeap OutItem;
	GetItems().RemoveAndCopyValue(Slot, OutItem);
	OnRemoveItem(OutItem, Slot);
	AddItem(Item, Slot);
	return OutItem;
}

bool IInventoryBase::SwapItemFrom(const TScriptInterface<IInventoryBase>& OutInventory, const int OutSlot, const int InSlot) {
	if (!OutInventory || !OutInventory->IsValidSlot(OutSlot) || !OutInventory->HasItem(OutSlot)  || !OutInventory->CanItemBeRemoved(OutSlot)) return false;
	if (AddItem(OutInventory->GetItems()[OutSlot].Top(), InSlot) < 0) return false;
	//should always be removed
	UItem* Top = OutInventory->GetItems()[OutSlot].Pop();
	check(Top);
	OutInventory->OnRemoveItem(Top, OutSlot);
	if (OutInventory->GetItems()[OutSlot].GetCount() <= 0)
		OutInventory->GetItems().Remove(OutSlot);
	return true;
}

bool IInventoryBase::SwapHeapFrom(const TScriptInterface<IInventoryBase>& OutInventory, const int OutSlot, const int InSlot) {
	if (!OutInventory || !OutInventory->IsValidSlot(OutSlot) || !OutInventory->HasItem(OutSlot)  || !OutInventory->CanItemBeRemoved(OutSlot)) return false;
	if (AddItem(OutInventory->GetItems()[OutSlot], InSlot) < 0) return false;
	//should always be removed
	FItemHeap OutItem;
	OutInventory->GetItems().RemoveAndCopyValue(OutSlot, OutItem);
	check(OutItem);
	OutInventory->OnRemoveItem(OutItem, OutSlot);
	return true;
}

bool IInventoryBase::ConsolidateItem(const FItemHeap& InItem, const int Slot) {
	if (!HasItem(Slot) || !CanItemBeInserted(InItem, Slot)) return false;
	FItemHeap& Item = GetItems()[Slot];
	if (Item && InItem == Item && Item.GetCount() < Item->GetStackAmount()) {
		for (UItem* ItemPtr : InItem.GetItems())
			Item.Push(ItemPtr);
		return true;
	}
	return false;
}

int IInventoryBase::AddItem_Internal(const FItemHeap Item, const int Slot) {
	if (!Item) return -1;
	const int StartSlot = IsValidSlot(Slot) ? Slot : 0;
	for (int s = StartSlot; s < StartSlot + GetCapacity(); s++) {
		const int i = s % GetCapacity(); 
		if (ConsolidateItem(Item, i))
			return i;
	}
	for (int s = StartSlot; s < StartSlot + GetCapacity(); s++) {
		const int i = s % GetCapacity(); 
		if (!HasItem(i) && CanItemBeInserted(Item, i)) {
			GetItems().Emplace(i, Item);
			return i;
		}
	}
	return -1;
}
