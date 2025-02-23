#pragma once

#include "Forested/ForestedMinimal.h"
#include "UObject/Interface.h"
#include "DamageableInterface.generated.h"

class AFPlayer;

UENUM(BlueprintType, meta = (ScriptName = "ForestedDamageType"))
enum class EDamageType : uint8 {
	Basic UMETA(DisplayName = "Basic"),
	Axe UMETA(DisplayName = "Axe"),
	Trowel UMETA(DisplayName = "Trowel"),
	Machete UMETA(DisplayName = "Machete"),
	StickLauncher UMETA(DisplayName = "Stick Launcher")
};
//ENUM_RANGE_BY_FIRST_AND_LAST(EDamageType, EDamageType::EDT_Basic, EDamageType::EDT_StickLauncher)

/*
This actor can be damaged by the player
overlap events will need to be enabled for this to work, as well as enabling the 'Hit' trace channel (internally known as ECC_GameTraceChannel5)
CanDamage needs to be called manually by the damage function otherwise it will do nothing, (useful for blueprint behaviors)
*/
UINTERFACE(MinimalAPI, Blueprintable)
class UDamageableInterface : public UInterface {
	GENERATED_BODY()
};

class FORESTED_API IDamageableInterface {
	GENERATED_BODY()

public:
	/*
	 * this is a base method used to apply damage to an actor, this is meant to only be implemented, not called, use UDamageLibrary::ApplyDamage
	 * note that HitResult.GetActor() would still be the child actor if it was the one that was hit
	 */
	UFUNCTION(BlueprintNativeEvent)
	void Damage(AFPlayer* Player, const FHitResult& HitResult, float Damage, EDamageType DamageType);
	virtual void Damage_Implementation(AFPlayer* Player, const FHitResult& HitResult, const float Damage, const EDamageType DamageType) {}

};

UCLASS()
class FORESTED_API UDamageLibrary : public UBlueprintFunctionLibrary {
	GENERATED_BODY()

public:

	/*
	 * a function to apply damage to an actor, if actor is a child actor the logic will be handled by its parent actor as they are components
	 * logic to check if it implements the interface does not need to be added and will break child actors
	 * note that HitResult.GetActor() would still be the child actor if it was the one that was hit
	 */
	UFUNCTION(BlueprintCallable, Category = "Damageable Interface")
	static void ApplyDamage(AActor* Actor, AFPlayer* Player, const FHitResult& HitResult, const float Damage, const EDamageType DamageType) {
		AActor* ActorToDamage = Actor;
		if (AActor* ParentActor = Actor->GetParentActor())
			ActorToDamage = ParentActor;
		if (ActorToDamage->GetClass()->ImplementsInterface(UDamageableInterface::StaticClass()))
			IDamageableInterface::Execute_Damage(ActorToDamage, Player, HitResult, Damage, DamageType);
	}
	
};