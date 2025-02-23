#pragma once

#include "Forested/ForestedMinimal.h"
#include "UObject/Interface.h"
#include "SelectableInterface.generated.h"

class AFPlayer;

UINTERFACE(MinimalAPI, Blueprintable)
class USelectableInterface : public UInterface {
	GENERATED_BODY()
};

/*
 * An interface for selecting objects
 * for an item to be selected, it must have a component with overlap events enabled that overlaps the visibility channel
 * Don't put this on child actors, as child actors are considered components and will be handled by the parent actor
 */
class FORESTED_API ISelectableInterface {
	GENERATED_BODY()

public:

	/*
	 * a function ran when the player selects an actor, if actor is a child actor the logic will be handled by its parent actor as they are components
	 * note that HitResult.GetActor() would still be the child actor if it was the one that was hit
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Selectable Interface")
	bool Selected(AFPlayer* Player, const FHitResult& HitResult, float HeldTime);
	virtual bool Selected_Implementation(AFPlayer* Player, const FHitResult& HitResult, const float HeldTime) { return false; }

	/*
	 * to test if the actor can be selected
	 * Default is HitResult.Distance <= 500.f
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Selectable Interface")
	bool CanSelect(const AFPlayer* Player, const FHitResult& HitResult, float HeldTime) const;
	virtual bool CanSelect_Implementation(const AFPlayer* Player, const FHitResult& HitResult, const float HeldTime) const {
		return HitResult.Distance <= 500.f;
	}
	
};

UINTERFACE(MinimalAPI, Blueprintable)
class UHoverableInterface : public UInterface {
	GENERATED_BODY()
};

/*
 * An interface for hovering over objects
 * for an item to be hovered, it must have a component with overlap events enabled that overlaps the visibility channel
 * Don't put this on child actors, as child actors are considered components and will be handled by the parent actor
 */
class FORESTED_API IHoverableInterface {
	GENERATED_BODY()

public:

	/*
	 * a function ran when the player hovers over an actor, if actor is a child actor the logic will be handled by its parent actor as they are components
	 * note that HitResult.GetActor() would still be the child actor if it was the one that was hit
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Selectable Interface")
	void Hovered(AFPlayer* Player, const FHitResult& HitResult);
	virtual void Hovered_Implementation(AFPlayer* Player, const FHitResult& HitResult) {}

	/*
	 * to test if the range and direction of the ray is valid to select
	 * Default is HitResult.Distance <= 500.f
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Selectable Interface")
	bool CanHover(const AFPlayer* Player, const FHitResult& HitResult) const;
	virtual bool CanHover_Implementation(const AFPlayer* Player, const FHitResult& HitResult) const {
		return HitResult.Distance <= 500.f;
	}
	
};
