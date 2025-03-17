#pragma once

#include "Forested/ForestedMinimal.h"
#include "Item.h"
#include "ShootItem.generated.h"

UCLASS()
class FORESTED_API UShootItem : public UItem {
	GENERATED_BODY()

public:

	UShootItem() {
	}

	virtual FItemProperties GetProperties_Implementation() const override {
		return FText::FromString("Gun Name");
	}

	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Shoot Item")
	int Bullets;
	
};
