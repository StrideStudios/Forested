#pragma once

#include "CoreMinimal.h"
#include "AxeHeadItem.h"
#include "AxeHandleItem.h"
#include "Item.h"
#include "AxeItem.generated.h"

/* comment from guy, could be important later though if I have no issues it doesn't really matter
Using CreateObject and assigning the resulting UObject to a variable in a Blueprint is not correct. The Generated UObject will be placed in the transient package while the instanced Blueprint will be in the Map’s package. The UObject reference cannot be saved because of the dependency and this is entirely expected.

If you want to create an object, you should have it’s outer be the Level. This will ensure that the instance of the Blueprint and the UObject exist in the same package when saving the map.
 */

UCLASS()
class FORESTED_API UAxeItem final : public UItem {
	GENERATED_BODY()

public:

	UAxeItem(): UAxeItem(UAxeHeadItem::StaticClass(), UAxeHandleItem::StaticClass()) {
	}

	UAxeItem(const TSubclassOf<UAxeHeadItem>& AxeHeadClass, const TSubclassOf<UAxeHandleItem>& AxeHandleClass):
	AxeHeadItem(nullptr),
	AxeHandleItem(nullptr),
	AxeHeadClass(AxeHeadClass),
	AxeHandleClass(AxeHandleClass) {
	}
	
	virtual FItemProperties GetProperties_Implementation() const override {
		return FText::FromString("Axe");
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Axe|Head")
	FORCEINLINE UAxeHeadItem* GetAxeHead() const { return AxeHeadItem; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Axe|Handle")
	FORCEINLINE UAxeHandleItem* GetAxeHandle() const { return AxeHandleItem; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Axe|Head")
	FORCEINLINE bool HasAxeHead() const { return IsValid(GetAxeHead()); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Axe|Handle")
	FORCEINLINE bool HasAxeHandle() const { return IsValid(GetAxeHandle()); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Axe|Head")
	FORCEINLINE TSubclassOf<UAxeHeadItem> GetAxeHeadClass() const { return AxeHeadClass; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Axe|Handle")
	FORCEINLINE TSubclassOf<UAxeHandleItem> GetAxeHandleClass() const { return AxeHandleClass; }
	/*
	virtual void Crafted(const TArray<UC_Item*>& Ingredients) override {
		Super::Crafted(Ingredients);
		SetAxeHead(Cast<UAxeHeadItem>(*Ingredients.FindByPredicate([](const UC_Item* Item) {
			return Item->IsA(UAxeHeadItem::StaticClass());
		})));
		SetAxeHandle(Cast<UAxeHandleItem>(*Ingredients.FindByPredicate([](const UC_Item* Item) {
			return Item->IsA(UAxeHandleItem::StaticClass());
		})));
	}
	*/
	UFUNCTION(BlueprintCallable, Category = "Axe|Head")
	FORCEINLINE bool SetAxeHead(UAxeHeadItem* AxeHead) {
		if (IsValid(AxeHeadItem)) return false;
		AxeHeadItem = AxeHead;
		return true;
	}
	
	UFUNCTION(BlueprintCallable, Category = "Axe|Handle")
	FORCEINLINE bool SetAxeHandle(UAxeHandleItem* AxeHandle) {
		if (IsValid(AxeHandleItem)) return false;
		AxeHandleItem = AxeHandle;
		return true;
	}

	UFUNCTION(BlueprintCallable, Category = "Axe|Head")
	FORCEINLINE UAxeHeadItem* RemoveAxeHead() {
		if (!IsValid(AxeHeadItem)) return nullptr;
		UAxeHeadItem* ReturnValue = AxeHeadItem;
		AxeHeadItem = nullptr;
		return ReturnValue;
	}

	UFUNCTION(BlueprintCallable, Category = "Axe|Handle")
	FORCEINLINE UAxeHandleItem* RemoveAxeHandle() {
		if (!IsValid(AxeHandleItem)) return nullptr;
		UAxeHandleItem* ReturnValue = AxeHandleItem;
		AxeHandleItem = nullptr;
		return ReturnValue;
	}

	UFUNCTION(BlueprintCallable, Category = "Axe")
	void OnAxeHit() const {
		AxeHeadItem->DecreaseDurability();
		AxeHeadItem->DecreaseSharpness();
	}

	virtual void Construction() override {
		if (!GetWorld()) return;
		AxeHeadItem = NewObject<UAxeHeadItem>(GetWorld(), GetAxeHeadClass(), "Axe Head");
		AxeHandleItem = NewObject<UAxeHandleItem>(GetWorld(), GetAxeHandleClass(), "Axe Handle");
	}
	
private:

	UPROPERTY()
	UAxeHeadItem* AxeHeadItem;
	
	UPROPERTY()
	UAxeHandleItem* AxeHandleItem;
	
	UPROPERTY(EditDefaultsOnly, Category = "Head")
	TSubclassOf<UAxeHeadItem> AxeHeadClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Handle")
	TSubclassOf<UAxeHandleItem> AxeHandleClass;
};