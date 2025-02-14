#pragma once

#include "Forested/ForestedMinimal.h"
#include "ObjectData.h"
#include "Item.generated.h"

class IInventoryBase;
class APlayerInventoryActor;

USTRUCT(BlueprintType)
struct FItemProperties {
	GENERATED_BODY()

	FItemProperties(const FText& ItemName = FText::FromString("Item Name"), const int StackAmount = 1, const TSoftObjectPtr<UTexture2D>& UiTexture = nullptr):
	ItemName(ItemName),
	StackAmount(StackAmount),
	UiTexture(UiTexture) {
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int StackAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> UiTexture;

};

UCLASS(BlueprintType, MinimalAPI, Blueprintable, EditInlineNew, DefaultToInstanced, HideDropdown, CollapseCategories)
class UItem : public UObject {
	GENERATED_BODY()

public:

	UItem() {
	}

	UFUNCTION(BlueprintCallable, Category = "Spawning", meta = (WorldContext="WorldContextObject", UnsafeDuringActorConstruction = "true", BlueprintInternalUseOnly = "true"))
	static UItem* ConstructItem(const UObject* WorldContextObject, TSubclassOf<UItem> ItemClass);

	UFUNCTION(BlueprintNativeEvent)
	void OnAddedToInventory(const TScriptInterface<IInventoryBase>& Inventory, int Slot);
	virtual void OnAddedToInventory_Implementation(const TScriptInterface<IInventoryBase>& Inventory, int Slot) {}
	
	FORCEINLINE bool operator==(const UItem& Item) const {
		return GetClass() == Item.GetClass();
	}

	FORCEINLINE virtual void PostInitProperties() override {
		Super::PostInitProperties();
		Construction();
		OnConstruction();
	}

	//called after properties have been initialized
	FORCEINLINE virtual void Construction() {
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item")
	static FItemProperties GetItemProperties(const TSubclassOf<UItem> Item) {
		return Item->GetDefaultObject<UItem>()->GetProperties();
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item")
	FORCEINLINE FText GetItemName() const {
		return GetProperties().ItemName;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item")
	FORCEINLINE int GetStackAmount() const {
		return GetProperties().StackAmount;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item")
	FORCEINLINE TSoftObjectPtr<UTexture2D> GetUiTexture() const {
		return GetProperties().UiTexture;
	}

protected:

	//called after properties have been initialized, this function is meant for blueprints
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Construction", Category = "Item")
	void OnConstruction();

	UFUNCTION(BlueprintNativeEvent, Category = "Item")
	FItemProperties GetProperties() const;
	virtual FItemProperties GetProperties_Implementation() const {
		return {};
	}

};

USTRUCT(BlueprintType)
struct FItemHeap {
	GENERATED_BODY()

friend class IInventoryBase;
	
	FItemHeap() = default;

	FItemHeap(UItem* Item):
	Items({ Item }) {
	}

	FItemHeap(const TArray<UItem*>& Items):
	Items(Items) {
	}
	
	/**
	* @returns Item Tags
	*/
	TArray<UItem*> GetItems() const {
		return Items;
	}

	/**
	* @returns Number of Item Tags
	*/
	int GetCount() const {
		return Items.Num();
	}

	/**
	 * @returns Valid Item If Num > 0
	 */
	UItem* Top() const {
		if (Items.Num() <= 0) return nullptr;
		return Items.Top();
	}

private:
	
	/**
	 * @param InItem Item Tag to add
	 */
	void Push(UItem* InItem) {
		Items.Push(InItem);
	}

	/**
	 * @returns Valid Item If Num > 0
	 */
	UItem* Pop() {
		if (Items.Num() <= 0) return nullptr;
		return Items.Pop();
	}

	/*
	 * Saves Data to ItemData
	 */
	void SaveItemHeap();
	
	/*
	 * Loads Data from ItemData
	 */
	void LoadItemHeap(UObject* Outer);

public:
	
	/**
	 * Equality operator.
	 * @param Heap Heap To Compare
	 * @returns True if the classes are equal
	 */
	bool operator==(const FItemHeap& Heap) const {
		check(Heap.Top());
		return Top()->GetClass() == Heap.Top()->GetClass();
	}
	
	/**
	 * If this heap is valid
	 * @returns True if Class is valid and heap has more than one
	 */
	FORCEINLINE operator bool() const {
		return Items.Num() > 0 && IsValid(Top());
	}

	/**
	 * @returns Valid Item from the Top of the Heap
	 */
	FORCEINLINE operator UItem*() const {
		return Top();
	}
	
	/**
	 * @returns Dereferenced Top Item
	 */
	FORCEINLINE UItem* operator*() const {
		return Top();
	}

	/**
	 * @returns Accessor to Dereferenced Top Item
	 */
	FORCEINLINE UItem* operator->() const {
		return Top();
	}

private:

	UPROPERTY(EditDefaultsOnly, Instanced) //TODO: Instanced wasnt needed in ue5, should find out why
	TArray<UItem*> Items = {};

	UPROPERTY(SaveGame)
	TArray<FObjectData> ItemData;
	
};

UCLASS()
class FORESTED_API UItemLibrary : public UBlueprintFunctionLibrary {
	GENERATED_BODY()

	/**
	 * @param Item Item to find the Class in
	 * @returns Class of the given item
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item|Tags")
	static FORCEINLINE TSubclassOf<UItem> GetClass(const FItemHeap& Item) {
		return Item->GetClass();
	}
	
	/**
	 * @param Item Item to find the Item Tags in
	 * @returns All Item Tags of given Item
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item|Tags")
	static FORCEINLINE TArray<UItem*> GetItems(const FItemHeap& Item) {
		return Item.GetItems();
	}

	/**
	 * @param Item Item to find the Count in
	 * @returns Number of Item Tags
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item|Tags")
	static FORCEINLINE int GetItemCount(const FItemHeap& Item) {
		return Item.GetCount();
	}

	/**
	 * @param Item to get the top element from
	 * @returns Valid Item If Num > 0
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item|Tags")
	static FORCEINLINE UItem* Top(const FItemHeap Item) {
		return Item.Top();
	}

	UFUNCTION(Blueprintcallable, BlueprintPure, meta = (DisplayName = "Equal (Item Heap)", CompactNodeTitle = "==", Keywords = "== equal"), Category = "Math|Item")
	static FORCEINLINE bool EqualEqual_ItemHeap(const FItemHeap& A, const FItemHeap& B) {
		return A == B;
	}
	
	UFUNCTION(Blueprintcallable, BlueprintPure, meta = (DisplayName = "IsValid (Item Heap)", Keywords = "Is Valid"), Category = "Math|Item")
	static FORCEINLINE bool IsValid_ItemHeap(const FItemHeap& A) {
		return A;
	}
};