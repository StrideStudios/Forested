#pragma once

#include "Forested/ForestedMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/SelectableInterface.h"
#include "Interfaces/DamageableInterface.h"
#include "Interfaces/LoadableInterface.h"
#include "Serialization/ObjectSaveGame.h"
#include "TreeActor.generated.h"

class ATreeChildActor;
class AFPlayer;

UCLASS(BlueprintType, meta = (PrioritizeCategories = "Tree Mesh"))
class FORESTED_API ATreeActor : public AActor, public ILoadableInterface , public IDamageableInterface, public ISelectableInterface, public ISaveActorInterface {
	GENERATED_BODY()
	
public:	
	ATreeActor();

protected:
	virtual void BeginPlay() override;

public:	

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tree Actor")
	FORCEINLINE TSubclassOf<ATreeChildActor> GetTreeChildActorClass() const { return TreeChildActorClass; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tree Actor")
	FORCEINLINE float GetHealth() const { return Health; }

	UFUNCTION(BlueprintCallable, Category = "Tree Actor")
	bool HasTreeChildActor() const;
	
	UFUNCTION(BlueprintCallable, Category = "Tree Actor")
	ATreeChildActor* GetTreeChildActor() const;
	
	UFUNCTION(BlueprintCallable, Category = "Tree Actor")
	bool IsStump() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tree Actor")
	bool IsSunk() const;
	
	virtual void Damage_Implementation(AFPlayer* Player, const FHitResult& HitResult, float Damage, EDamageType DamageType) override;

	virtual bool Selected_Implementation(AFPlayer* Player, const FHitResult& HitResult, float HeldTime) override;
	
	virtual void OnGameSave_Implementation(UObject* WorldContextObject, UObjectSaveGame* SaveGame) override;

	virtual void OnGameLoad_Implementation(UObject* WorldContextObject, UObjectSaveGame* SaveGame) override;

	virtual void LoadDefaults_Implementation(UObject* WorldContextObject) override;

	virtual bool CanLoadTick_Implementation() override {
		return true;
	}
	
	virtual void LoadTick_Implementation() override;
	
	virtual void Load_Implementation() override;
	
	virtual void Unload_Implementation() override;

	UFUNCTION(BlueprintCallable, Category = "Tree Actor")
	void SetTreeFelled(const FVector& FallDirection);
	
	UFUNCTION(BlueprintCallable, Category = "Tree Actor")
	void TreeSpawned();

	UFUNCTION(BlueprintCallable, Category = "Tree Actor")
	void ClearTreeChildActor(bool DestroyStump = false);

	UFUNCTION(BlueprintCallable, Category = "Tree Actor")
	void SetTreeChildActor(TSoftClassPtr<ATreeChildActor> InTreeActorClass);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	USceneComponent* Root;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	UChildActorComponent* TreeChildActorComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	UStaticMeshComponent* DirtComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	UStaticMeshComponent* StumpComponent;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tree")
	TSoftClassPtr<ATreeChildActor> DefaultTreeChildActorClass;
	
	UPROPERTY(SaveGame)
	TSubclassOf<ATreeChildActor> TreeChildActorClass;

	UPROPERTY(SaveGame)
	FObjectData TreeChildData;

	UPROPERTY(SaveGame)
	float Health = 100.f;

	UPROPERTY(SaveGame)
	float StumpScale;

private:
	
	void SetTreeChildActor_Internal(const TSubclassOf<ATreeChildActor>& InTreeActorClass);
	
	UPROPERTY()
	ATreeChildActor* TreeChildActor;

	FVector SetPosition;
};

//
/*
 * Contains the data used for a seed, properties are assumed non-null
 */

UINTERFACE()
class USeedInterface : public UInterface {
	GENERATED_BODY()
};

class ISeedInterface {
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent)
	void OnSeedPlanted(ATreeActor* TreeActor);
	virtual void OnSeedPlanted_Implementation(ATreeActor* TreeActor) {}

	UFUNCTION(BlueprintNativeEvent)
	TSoftClassPtr<ATreeChildActor> GetTreeActorClass();
	virtual TSoftClassPtr<ATreeChildActor> GetTreeActorClass_Implementation() {
		return nullptr;
	}
	
};