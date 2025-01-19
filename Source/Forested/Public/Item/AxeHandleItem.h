#pragma once

#include "CoreMinimal.h"
#include "RankedWoodItem.h"
#include "AxeHandleItem.generated.h"

UCLASS()
class FORESTED_API UAxeHandleItem final : public URankedWoodItem {
	GENERATED_BODY()

public:

	UAxeHandleItem() {
	}

	virtual FItemProperties GetProperties_Implementation() const override {
		return FText::FromString("Axe Handle");
	}

};