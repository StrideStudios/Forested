#pragma once

#include "Forested/ForestedMinimal.h"
#include "Item.h"
#include "SeedItem.generated.h"

UCLASS()
class FORESTED_API USeedItem : public UItem {
	GENERATED_BODY()

public:

	USeedItem() {
	}

	virtual FItemProperties GetProperties_Implementation() const override {
		return FText::FromString("Seed Name");
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftClassPtr<class ATreeChildActor> TreeClass;
	
};
