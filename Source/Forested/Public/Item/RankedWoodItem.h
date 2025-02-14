#pragma once

#include "Forested/ForestedMinimal.h"
#include "RankedItem.h"
#include "RankedWoodItem.generated.h"

UCLASS(BlueprintType, Blueprintable)
class FORESTED_API UWoodRank : public URank {
	GENERATED_BODY()

public:

	UWoodRank():
	UWoodRank(0, "Wood Rank") {
	}

	UWoodRank(const int Rank, const FName Name):
	URank(Rank, Name) {
	}
};

UCLASS()
class FORESTED_API URankedWoodItem : public URankedItem {
	GENERATED_BODY()

public:

	URankedWoodItem():
	URankedWoodItem(UWoodRank::StaticClass()) {
	}

	explicit URankedWoodItem(const TSubclassOf<UWoodRank>& DefaultRank):
	DefaultRank(DefaultRank) {
	}

	virtual FItemProperties GetProperties_Implementation() const override {
		return FText::FromString("Ranked Wood Item Name");
	}

	virtual void Construction() override {
		SetRank(DefaultRank);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Wood Rank Item")
	UWoodRank* GetWoodRank() const {
		return CastChecked<UWoodRank>(GetRank());
	}

	UFUNCTION(BlueprintCallable, Category = "Wood Rank Item")
	void SetWoodRank(const TSubclassOf<UWoodRank> WoodRank) {
		SetRank(WoodRank);
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wood Rank Item")
	TSubclassOf<UWoodRank> DefaultRank;

};