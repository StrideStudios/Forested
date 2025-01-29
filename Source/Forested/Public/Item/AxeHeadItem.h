#pragma once

#include "CoreMinimal.h"
#include "RankedMetalItem.h"
#include "AxeHeadItem.generated.h"

UCLASS(meta = (PrioritizeCategories ="Properties"))
class FORESTED_API UAxeHeadItem final : public URankedMetalItem {
	GENERATED_BODY()

public:

	//default blueprint constructor
	UAxeHeadItem(): UAxeHeadItem(1.f, 1.f, 1.f, 1.f) {
	}

	//a c++ only constructor to allow setting of default variables
	UAxeHeadItem(const float BaseDamage, const float BaseSpeed, const float BaseDurabilityDegradation, const float BaseSharpnessDegradation): 
	BaseDamage(BaseDamage),
	BaseSpeed(BaseSpeed),
	BaseDurabilityDegradation(BaseDurabilityDegradation),
	BaseSharpnessDegradation(BaseSharpnessDegradation),
	Durability(100.f),
	Sharpness(100.f){
	}

	virtual FItemProperties GetProperties_Implementation() const override {
		return FText::FromString("Axe Head");
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Axe Head")
	FORCEINLINE float GetDamage() const {
		return BaseDamage * GetMetalRank()->GetDamageMultiplier();
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Axe Head")
	FORCEINLINE float GetSpeed() const {
		return BaseSpeed * GetMetalRank()->GetSpeedMultiplier();
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Axe Head")
	FORCEINLINE float GetDurability() const {
		return Durability;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Axe Head")
	FORCEINLINE float GetSharpness() const {
		return Sharpness;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Axe Head")
	FORCEINLINE float GetDurabilityDegradation() const {
		return BaseDurabilityDegradation * GetMetalRank()->GetDurabilityDegradationMultiplier();
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Axe Head")
	FORCEINLINE float GetSharpnessDegradation() const {
		return BaseSharpnessDegradation * GetMetalRank()->GetSharpnessDegradationMultiplier();
	}

	UFUNCTION(BlueprintCallable, Category = "Axe Head")
	void DecreaseDurability() {
		Durability = Durability - GetDurabilityDegradation();
	}

	UFUNCTION(BlueprintCallable, Category = "Axe Head")
	void DecreaseSharpness() {
		Sharpness = Sharpness - GetSharpnessDegradation();
	}

	UFUNCTION(BlueprintCallable, Category = "Axe Head")
	void IncreaseSharpness(const float SharpenAmount) {
		Sharpness = Sharpness + SharpenAmount;
	}
	
private:

	//The base number for how much damage this axe does
	UPROPERTY(EditAnywhere, Category = "Properties")
	float BaseDamage;

	//The base number for how quickly the axe swings (also affected by handle speed)
	UPROPERTY(EditAnywhere, Category = "Properties")
	float BaseSpeed;

	//The base number for durability degradation, durability starts at 100 and degrades by this number every hit (can be a decimal)
	UPROPERTY(EditAnywhere, Category = "Properties")
	float BaseDurabilityDegradation;

	//The base number for sharpness degradation, sharpness starts at 100 and degrades by this number every hit (can be a decimal)
	UPROPERTY(EditAnywhere, Category = "Properties")
	float BaseSharpnessDegradation;
	
	UPROPERTY(SaveGame)
	float Durability;//sturdiness
	
	UPROPERTY(SaveGame)
	float Sharpness;
	
};