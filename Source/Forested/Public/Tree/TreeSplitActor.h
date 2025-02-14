#pragma once

#include "Forested/ForestedMinimal.h"
#include "GameFramework/Actor.h"
#include "DamageableInterface.h"
#include "LoadableInterface.h"
#include "ObjectSaveGame.h"
#include "TreeChildComponent.h"
#include "Item/Item.h"
#include "TreeSplitActor.generated.h"

class UBoxComponent;
class AFPlayer;
class UObjectSaveGame;

USTRUCT(BlueprintType)
struct FSplitLocationData {
	GENERATED_BODY()

	FSplitLocationData() :
	FSplitLocationData(nullptr, {}, nullptr, FTransform::Identity) {
	}

	FSplitLocationData(const TSubclassOf<UTrunkComponent>& Class, const FObjectData& ItemData, const TSoftObjectPtr<UStaticMesh>& StaticMesh, const FTransform& Transform):
	Class(Class),
	ItemData(ItemData),
	StaticMesh(StaticMesh),
	Transform(Transform) {
	}

	UPROPERTY(SaveGame)
	TSubclassOf<UTrunkComponent> Class;

	UPROPERTY(SaveGame)
	FObjectData ItemData;

	UPROPERTY(SaveGame)
	TSoftObjectPtr<UStaticMesh> StaticMesh;

	UPROPERTY(SaveGame)
	FTransform Transform;

};

UCLASS()//, public ILoadRangeInterface
class FORESTED_API ATreeSplitActor : public AActor, public ISaveActorInterface, public IDamageableInterface, public ILoadableInterface {
	GENERATED_BODY()
	
public:	
	ATreeSplitActor();

	virtual void OnGameSave_Implementation(UObject* WorldContextObject, UObjectSaveGame* SaveGame) override;

	virtual void OnGameLoad_Implementation(UObject* WorldContextObject, UObjectSaveGame* SaveGame) override;

	virtual void Load_Implementation() override;

	virtual void Unload_Implementation() override;

	virtual void Damage_Implementation(AFPlayer* Player, const FHitResult& HitResult, float Damage, EDamageType DamageType) override;

	//virtual void UpdateLoadRange_Implementation(ELoadRange InLoadRange, bool Value) override;

	//for this function to work properly, go from the lowest child component first to the parent component
	UFUNCTION(BlueprintCallable, Category = "Tree Split Actor")
	void AddTreeComponents(TArray<UTrunkComponent*> InTrunkComponents);

	void AddImpulse(const FVector& Impulse);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USceneComponent* Root;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UBoxComponent* BoxComponent;
	
private:

	UTrunkComponent* CreateComponent(UClass* Class, UTreeItem* Item, const TSoftObjectPtr<UStaticMesh>& StaticMesh, const FTransform& Transform);

	void AttachComponents();

	static void SetComponentSimulatePhysics(const UPrimitiveComponent* Component, bool Simulate);

	void UpdateSphereBounds();
	
	UPROPERTY(SaveGame)
	TMap<uint8, FSplitLocationData> LocationData;

	UPROPERTY()
	TArray<UTrunkComponent*> TrunkComponents;

	FVector LinearVelocity = FVector::ZeroVector;
	
	FVector AngularVelocity = FVector::ZeroVector;
	
};
