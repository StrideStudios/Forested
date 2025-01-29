#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "RankedItem.generated.h"

UCLASS(Blueprintable)
class FORESTED_API URank : public UObject {
	GENERATED_BODY()

public:

	URank(): 
		URank(0, "Rank") {
	}

	URank(const int Rank, const FName Name):
		Rank(Rank),
		Name(Name) {
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Rank")
	FORCEINLINE int GetRank() const { return Rank; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Rank")
	FORCEINLINE FName GetRankName() const { return Name; }

private:

	UPROPERTY(EditDefaultsOnly)
	int Rank;

	UPROPERTY(EditDefaultsOnly)
	FName Name;
};

UCLASS()
class FORESTED_API URankedItem : public UItem {
	GENERATED_BODY()

public:

	URankedItem() {
	}

	virtual FItemProperties GetProperties_Implementation() const override {
		return FText::FromString("Ranked Item Name");
	}

	bool operator==(const URankedItem& RankedItem) const {
		return GetClass() == RankedItem.GetClass() && GetRank() == RankedItem.GetRank();
	}

	//UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item|Rank", meta = (BlueprintProtected))
	FORCEINLINE URank* GetRank() const { return CastChecked<URank>(Rank->GetDefaultObject()); }

	FORCEINLINE TSubclassOf<URank> GetRankClass() const { return Rank; }
	
	FORCEINLINE void SetRank(const TSubclassOf<URank>& InRank) {
		Rank = InRank;
	}

private:

	/*
	used to return a rank that this item is
	its best to have a Class Variable and get its default object
	see UMetalRank for an example
	*/
	/*
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintNativeEvent, Category = "Item|Rank")
	URank* GetRank() const;
	virtual URank* GetRank_Implementation() const {
		return nullptr;
	}
	*/

	TSubclassOf<URank> Rank;

};


