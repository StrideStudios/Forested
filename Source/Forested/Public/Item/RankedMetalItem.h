#pragma once

#include "CoreMinimal.h"
#include "RankedItem.h"
#include "RankedMetalItem.generated.h"

UCLASS(BlueprintType, Blueprintable)
class FORESTED_API UMetalRank : public URank {
	GENERATED_BODY()

public:

	UMetalRank(): 
	UMetalRank(0,"Metal Rank", 1.f, 1.f, 1.f, 1.f) {
	}

	UMetalRank(const int Rank, const FName Name, const float DamageMultiplier, const float SpeedMultiplier, const float DurabilityDegradationMultiplier, const float SharpnessDegradationMultiplier): 
	URank(Rank,Name), 
	DamageMultiplier(DamageMultiplier), 
	SpeedMultiplier(SpeedMultiplier), 
	DurabilityDegradationMultiplier(DurabilityDegradationMultiplier), 
	SharpnessDegradationMultiplier(SharpnessDegradationMultiplier) {
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Metal Rank")
	FORCEINLINE float GetDamageMultiplier() const { return DamageMultiplier; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Metal Rank")
	FORCEINLINE float GetSpeedMultiplier() const { return SpeedMultiplier; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Metal Rank")
	FORCEINLINE float GetDurabilityDegradationMultiplier() const { return DurabilityDegradationMultiplier; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Metal Rank")
	FORCEINLINE float GetSharpnessDegradationMultiplier() const { return SharpnessDegradationMultiplier; }

private:

	//how much damage this rank adds
	UPROPERTY(EditDefaultsOnly)
	float DamageMultiplier;

	//how much speed this rank adds
	UPROPERTY(EditDefaultsOnly)
	float SpeedMultiplier;

	//how quickly durability decreases
	UPROPERTY(EditDefaultsOnly)
	float DurabilityDegradationMultiplier;

	//how quickly sharpness decreases
	UPROPERTY(EditDefaultsOnly)
	float SharpnessDegradationMultiplier;

};

UCLASS(NotBlueprintable)
class FORESTED_API UCopperRank final : public UMetalRank {
	GENERATED_BODY()

	UCopperRank(): 
	UMetalRank(1, "Copper", 1.f, 1.f, 1.f, 1.f) {
	}
};

UCLASS(NotBlueprintable)
class FORESTED_API UTinRank final : public UMetalRank {
	GENERATED_BODY()

	UTinRank():
	UMetalRank(1, "Tin", 1.f, 1.f, 1.f, 1.f) {
	}
};

UCLASS(NotBlueprintable)
class FORESTED_API UBronzeRank final : public UMetalRank {
	GENERATED_BODY()

	UBronzeRank():
	UMetalRank(1, "Bronze", 1.f, 1.f, 1.f, 1.f) {
	}
};

UCLASS(NotBlueprintable)
class FORESTED_API UIronRank final : public UMetalRank {
	GENERATED_BODY()

	UIronRank():
	UMetalRank(1, "Iron", 1.f, 1.f, 1.f, 1.f) {
	}
};

UCLASS(NotBlueprintable)
class FORESTED_API UCopperIronRank final : public UMetalRank {
	GENERATED_BODY()

	UCopperIronRank():
	UMetalRank(1, "Copper_Iron", 1.f, 1.f, 1.f, 1.f) {
	}
};

UCLASS(NotBlueprintable)
class FORESTED_API UTinIronRank final : public UMetalRank {
	GENERATED_BODY()

	UTinIronRank():
	UMetalRank(1, "Tin_Iron", 1.f, 1.f, 1.f, 1.f) {
	}
};

UCLASS(NotBlueprintable)
class FORESTED_API UBronzeIronRank final : public UMetalRank {
	GENERATED_BODY()

	UBronzeIronRank():
	UMetalRank(1, "Bronze_Iron", 1.f, 1.f, 1.f, 1.f) {
	}
};

UCLASS()
class FORESTED_API URankedMetalItem : public URankedItem {
	GENERATED_BODY()

public:

	URankedMetalItem():
	URankedMetalItem(UMetalRank::StaticClass()) {
	}

	explicit URankedMetalItem(const TSubclassOf<UMetalRank>& DefaultRank):
	DefaultRank(DefaultRank) {
	}

	virtual FItemProperties GetProperties_Implementation() const override {
		return FText::FromString("Ranked Metal Item Name");
	}
	
	virtual void Construction() override {
		SetRank(DefaultRank);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Metal Rank Item")
	UMetalRank* GetMetalRank() const {
		return Cast<UMetalRank>(GetRank());
	}

	UFUNCTION(BlueprintCallable, Category = "Metal Rank Item")
	void SetMetalRank(const TSubclassOf<UMetalRank> MetalRank) {
		SetRank(MetalRank);
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Metal Rank Item")
	TSubclassOf<UMetalRank> DefaultRank;

};