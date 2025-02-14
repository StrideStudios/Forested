#pragma once

#include "Forested/ForestedMinimal.h"
#include "Item.h"
#include "ObjectSaveGame.h"
#include "SelectableInterface.h"
#include "Inventory.generated.h"

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UInventoryBase : public USaveObjectInterface {
	GENERATED_BODY()
};

class FORESTED_API IInventoryBase : public ISaveObjectInterface {
	GENERATED_BODY()

public:

	/*
	 * Functions to Inherit
	 * because Items need to be saved, they need to be in a UObject
	 */

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual int GetCapacity() const = 0;

protected:

	virtual UObject* GetObject() {
		return dynamic_cast<UObject*>(this);
	}
	
	virtual const UObject* GetObject() const {
		return dynamic_cast<const UObject*>(this);
	}
	
	virtual TMap<int32, FItemHeap>& GetItems() = 0;

	virtual const TMap<int32, FItemHeap>& GetItems() const = 0;
	
	virtual void OnInsertItem(FItemHeap Item, const int Slot) = 0;
	
	virtual void OnRemoveItem(FItemHeap Item, const int Slot) = 0;

	virtual void OnAppendItems(const TMap<int32, FItemHeap>& AppendedItems) = 0;

	//function to limit what items can't be added to inventories
	virtual bool CanItemBeInserted(FItemHeap Item, const int Slot) {
		return true;
	}

	//function to limit what items can't be removed from inventories
	virtual bool CanItemBeRemoved(const int Slot) {
		return true;
	}

public:
	
	/*
	 * Inherited Functions
	 */

	virtual void OnGameSave_Implementation(UObject* WorldContextObject, UObjectSaveGame* SaveGame) override;

	virtual void OnGameLoad_Implementation(UObject* WorldContextObject, UObjectSaveGame* SaveGame) override;
	
	/*
	 * Base Functions
	 */

	/**
	 * @param Slot the slot to check
	 * @return true if the slot can possibly exist
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual FORCEINLINE bool IsValidSlot(const int Slot) const { return Slot >= 0 && Slot < GetCapacity(); }
	
	/**
	 * @param Slot the slot to check
	 * @return true if this inventory has an item in the given slot
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual FORCEINLINE bool HasItem(const int Slot) const { return GetItems().Contains(Slot) && GetItems()[Slot]; }
	
	/** Function to get items, returns a value in a way which prevents unwanted editing of items
	 * @param Slot the slot to get the item from
	 * @param OutItem item from inventory
	 * @return true if item is valid
	 */
	FORCEINLINE bool GetItem(const int Slot, FItemHeap& OutItem) const {
		if (!HasItem(Slot)) return false;
		OutItem = GetItems()[Slot];
		return true;
	}
	
	/*
	 * Addition/Removal of Items
	 */

	/** insert an item heap into a slot, will also consolidate
	 * @param Item the heap to insert
	 * @param Slot the slot to insert into
	 * @return true if an item was successfully inserted into this inventory
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Insert Heap", Category = "Inventory")
	virtual bool InsertItem(const FItemHeap Item, const int Slot);

	/** will remove a single item from the inventory, this does not guarantee a slot has been freed, see RemoveHeap
	 * @param Slot the slot to remove the item
	 * @return valid item if successfully removed from a slot
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual UItem* RemoveItem(const int Slot);

	/** will remove an item heap from the inventory, this guarantees a slot will be available
	 * @param Slot the slot to remove the item
	 * @return valid item heap if successfully removed from a slot
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual FItemHeap RemoveHeap(const int Slot);

	/** add an item heap into a slot, will add to empty slots and also consolidate
	 * @param Item the heap to add
	 * @param Slot the slot to add into
	 * @return >= 0 if an item was successfully added into this inventory, -1 if failed
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Add Heap", Category = "Inventory")
	virtual int AddItem(const FItemHeap Item, const int Slot = -1);

	/** append a list of item heaps into this inventory, will attempt to add all but will return the heaps that failed
		 * @param Items the heaps to append
		 * @param OutItems the items that could not be added
		 * @return true if all items were successfully appended
		 */
	UFUNCTION(BlueprintCallable, DisplayName = "Insert Heap", Category = "Inventory")
	virtual bool AppendItems(const TArray<FItemHeap>& Items, TArray<FItemHeap>& OutItems);
	
	/** will replace and item heap with another item heap, meaning the heap has been fully removed and replaced
	 * @param Item the item heap to replace with
	 * @param Slot the slot to replace into
	 * @return a valid item heap if an item was successfully replaced
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Replace Heap", Category = "Inventory")
	virtual FItemHeap ReplaceItem(const FItemHeap Item, const int Slot);

	/** Swap Items from the OutInventory to this via the item in OutSlot, this uses add, so the first available slot will be taken
	 * This only swaps one item from a heap, so this does not guarantee an empty slot
	 * @param OutInventory the inventory to remove from
	 * @param OutSlot the slot to remove from
	 * @param InSlot the slot to add into
	 * @return true if the item was successfully swapped
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool SwapItemFrom(const TScriptInterface<IInventoryBase>& OutInventory, const int OutSlot, const int InSlot = -1);
	
	/** Swap Items from the OutInventory to this via the item in OutSlot, this uses add, so the first available slot will be taken
	 * This swaps the whole item heap, so an empty slot is guaranteed
	 * @param OutInventory the inventory to remove from
	 * @param OutSlot the slot to remove from
	 * @param InSlot the slot to add into
	 * @return true if the item was successfully swapped
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool SwapHeapFrom(const TScriptInterface<IInventoryBase>& OutInventory, const int OutSlot, const int InSlot = -1);
	
protected:

    UFUNCTION(BlueprintCallable, DisplayName = "Get Item", Category = "Inventory")
    virtual FORCEINLINE bool ReceiveGetItem(const int Slot, FItemHeap& OutItem) const {
    	return GetItem(Slot, OutItem);
    }

	/*
	 * functions that allow blueprints to use single items without conversion
	 */

	/** insert an item into a slot, will also consolidate
	 * @param Item the item to insert
	 * @param Slot the slot to insert into
	 * @return true if an item was successfully inserted into this inventory
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Insert Item", Category = "Inventory")
	virtual bool InsertItemPtr(UItem* Item, const int Slot) {
	    return InsertItem(Item, Slot);
    }

	/** add an item into a slot, will add to empty slots and also consolidate
	 * @param Item the item to add
	 * @param Slot the slot to add into
	 * @return >= 0 if an item was successfully added into this inventory, -1 if failed
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Add Item", Category = "Inventory")
	virtual int AddItemPtr(UItem* Item, const int Slot = -1) {
		return AddItem(Item, Slot);
	}

	/** will replace and item with another item, meaning the item has been fully removed and replaced
	 * @param Item the item to replace with
	 * @param Slot the slot to replace into
	 * @return a valid item heap if an item was successfully replaced
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Replace Item", Category = "Inventory")
	virtual FItemHeap ReplaceItemPtr(UItem* Item, const int Slot) {
	    return ReplaceItem(Item, Slot);
    }

private:
	
	bool ConsolidateItem(const FItemHeap& InItem, int Slot);

	int AddItem_Internal(const FItemHeap Item, const int Slot = -1);

};

/*
 * Interface used to define blueprint functions
 */

UINTERFACE(MinimalAPI, Blueprintable)
class UInventory : public UInventoryBase {
	GENERATED_BODY()
};

class FORESTED_API IInventory : public IInventoryBase {
	GENERATED_BODY()

protected:

	virtual bool CanItemBeInserted(const FItemHeap Item, const int Slot) override {
		return Execute_ReceiveCanItemBeInserted(GetObject(), Item, Slot);
	}

	virtual bool CanItemBeRemoved(const int Slot) override {
		return Execute_ReceiveCanItemBeRemoved(GetObject(), Slot);
	}

	virtual void OnInsertItem(const FItemHeap Item, const int Slot) override {
		Execute_ReceiveOnInsertItem(GetObject(), Item, Slot);
	}

	virtual void OnRemoveItem(const FItemHeap Item, const int Slot) override {
		Execute_ReceiveOnRemoveItem(GetObject(), Item, Slot);
	}

	virtual void OnAppendItems(const TMap<int32, FItemHeap>& AppendedItems) override {
		Execute_ReceiveOnAppendItems(GetObject(), AppendedItems);
	}

	UFUNCTION(BlueprintNativeEvent, DisplayName = "Can Item Be Inserted", Category = "Inventory")
	bool ReceiveCanItemBeInserted(FItemHeap Item, const int Slot);
	bool ReceiveCanItemBeInserted_Implementation(FItemHeap Item, const int Slot) { return true; }
	
	UFUNCTION(BlueprintNativeEvent, DisplayName = "Can Item Be Removed", Category = "Inventory")
	bool ReceiveCanItemBeRemoved(const int Slot);
	bool ReceiveCanItemBeRemoved_Implementation(const int Slot) { return true; }

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Insert", Category = "Inventory")
	void ReceiveOnInsertItem(FItemHeap Item, const int Slot);
	
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Remove", Category = "Inventory")
	void ReceiveOnRemoveItem(FItemHeap Item, const int Slot);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Append", Category = "Inventory")
	void ReceiveOnAppendItems(const TMap<int32, FItemHeap>& AppendedItems);
	
};