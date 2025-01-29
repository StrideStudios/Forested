#pragma once

#include "CoreMinimal.h"
#include "SelectableInterface.h"
#include "LoadableInterface.h"
#include "ObjectSaveGame.h"
#include "GameFramework/Actor.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "ItemActor.generated.h"

class USphereComponent;
class UCullStaticMeshComponent;
class UItem;

DECLARE_DELEGATE_OneParam(FItemLoadDelegate, class AItemActor*);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FItemLoadDynamicDelegate, AItemActor*, ItemActor);

//Placement should be done via calling made functions
UCLASS(NotPlaceable, meta = (PrioritizeCategories = "Mesh"))
class FORESTED_API AItemActor : public AActor, public ISelectableInterface, public ISaveActorInterface, public ILoadableInterface {
	GENERATED_BODY()

	friend class UItemActorSpawnAsyncAction;

protected:

	virtual void BeginPlay() override;
	
public:	
	AItemActor();

	UFUNCTION(BlueprintCallable, Category = "Render Actor")
	virtual void Init();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, DisplayName = "Init", Category = "Render Actor")
	void ReceiveInit();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item Actor")
	FORCEINLINE UItem* GetItem() const { return Item; }
	
	virtual bool Selected_Implementation(AFPlayer* Player, const FHitResult& HitResult, float HeldTime) override;

	virtual void OnGameSave_Implementation(UObject* WorldContextObject, UObjectSaveGame* SaveGame) override;
	
	virtual void OnGameLoad_Implementation(UObject* WorldContextObject, UObjectSaveGame* SaveGame) override;

	virtual void Load_Implementation() override;

	virtual void Unload_Implementation() override;

	//virtual void RefreshMeshBounds(float CullDistanceOverride = 0) override;
	
	//virtual void SetCullDistance(UPrimitiveComponent* Component, float CullDistance = 0.f) override;

	static bool SpawnItemActor(const UObject* WorldContextObject, const FTransform& Transform, UItem* Item, const FVector& InLinearVelocity = FVector(0.f), const FVector& InAngularVelocity = FVector(0.f)) {
		return SpawnItemActor_Internal(WorldContextObject, Transform, Item, {}, InLinearVelocity, InAngularVelocity);
	}
	
	template<class UserClass>
	static bool SpawnItemActor(const UObject* WorldContextObject, const FTransform& Transform, UItem* Item, UserClass* InObj, FItemLoadDelegate::TUObjectMethodDelegate<UserClass> DelegateToCall, const FVector& InLinearVelocity = FVector(0.f), const FVector& InAngularVelocity = FVector(0.f)) {
		return SpawnItemActor_Internal(WorldContextObject, Transform, Item, FItemLoadDelegate::CreateUObject(InObj, DelegateToCall), InLinearVelocity, InAngularVelocity);
	}
	
	static bool SpawnItemActor(const UObject* WorldContextObject, const FTransform& Transform, UItem* Item, const TFunction<void(AItemActor*)>& Function, const FVector& InLinearVelocity = FVector(0.f), const FVector& InAngularVelocity = FVector(0.f)) {
		return SpawnItemActor_Internal(WorldContextObject, Transform, Item, FItemLoadDelegate::CreateLambda(Function), InLinearVelocity, InAngularVelocity);
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UCullStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USphereComponent* SphereComponent;
	
protected:

	UPROPERTY()
	UItem* Item;
	
	UPROPERTY(SaveGame)
	FObjectData ItemData;

private:

	static bool SpawnItemActor_Internal(const UObject* WorldContextObject, const FTransform& Transform, UItem* Item, FItemLoadDelegate OnItemLoadComplete, FVector InLinearVelocity = FVector(0.f), FVector InAngularVelocity = FVector(0.f));

	UPROPERTY()
	UStaticMesh* StaticMesh = nullptr;

	FVector LinearVelocity = FVector::ZeroVector;
	
	FVector AngularVelocity = FVector::ZeroVector;

	FItemLoadDelegate OnItemLoadComplete;
	
};

USTRUCT(BlueprintType)
struct FItemActorProperties {
	GENERATED_BODY()

	FItemActorProperties() {
	}

	//the class to use, leave default if extra logic is not needed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, NoClear)
	TSoftClassPtr<AItemActor> ItemActorClass = AItemActor::StaticClass();

	//whether to override the static mesh set in the item actor
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, AdvancedDisplay)
	bool bOverrideStaticMesh = false;
	
	//The mesh used to render this item
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, AdvancedDisplay)
	TSoftObjectPtr<UStaticMesh> StaticMesh = nullptr;

	//whether to override the default mass setting of the actor
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, AdvancedDisplay)
	bool bOverrideMass = false;
	
	//the override mass of the actor
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, AdvancedDisplay, meta = (ClampMin = "0.001", UIMin = "0.001", DisplayName = "Mass (kg)"))
	float MassInKgOverride = 0.f;
	
	//the override cull distance of the actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, meta = (ClampMin = "0", UIMin = "0"))
	float CullScale = 25.f;

	//how much drag to add when the actor is being translated
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay)
	float LinearDrag = 0.05f;

	//how much drag to add when the actor is being rotated
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay)
	float AngularDrag = 0.25f;
};

UINTERFACE()
class UItemActorInterface : public UInterface {
	GENERATED_BODY()
};

class IItemActorInterface {
	GENERATED_BODY()

public:

	/**
	 * Event that fires when an item actor is spawned with this item
	 * @param ItemActor The item actor that was spawned
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Item Actor Interface")
	void OnAddedToItemActor(AItemActor* ItemActor);
	virtual void OnAddedToItemActor_Implementation(AItemActor* ItemActor) {}

	/**
	 * Get Properties for the item actor
	 * @result The properties for the item
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Item Actor Interface")
	FItemActorProperties GetItemActorProperties() const;
	virtual FItemActorProperties GetItemActorProperties_Implementation() const {
		return {};
	}
	
};

UCLASS()
class FORESTED_API UItemActorSpawnAsyncAction : public UBlueprintAsyncActionBase {
	GENERATED_BODY()

public:

	/*
	 * this is the best place for property overrides because I am too lazy to make another class
	 */

	UFUNCTION(BlueprintCallable, Category = "Item Properties")
	static FItemActorProperties SetOverrideStaticMesh(UPARAM(ref) FItemActorProperties& Properties, TSoftObjectPtr<UStaticMesh> StaticMesh);
	
	UFUNCTION(BlueprintCallable, Category = "Item Properties")
	static FItemActorProperties SetOverrideMass(UPARAM(ref) FItemActorProperties& Properties, float MassInKgOverride);
	
protected:

	/*
	 * parameters
	 */

	UObject* Object;

	FTransform Transform;

	UItem* Item;

	FVector LinearVelocity;
	
	FVector AngularVelocity;

	UPROPERTY(BlueprintAssignable)
	FItemLoadDynamicDelegate Complete;

	UPROPERTY(BlueprintAssignable)
	FItemLoadDynamicDelegate Failed;

	/*
	 * functions
	 */

	//Spawn an item actor in the world, completes once the static mesh is loaded
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly = "true", WorldContext="WorldContextObject", AdvancedDisplay = 3), Category = "Item Actor|Spawning")
	static UItemActorSpawnAsyncAction* SpawnItemActor(UObject* WorldContextObject, FTransform Transform, UItem* Item, FVector InLinearVelocity = FVector(0.f), FVector InAngularVelocity = FVector(0.f));

	virtual void Activate() override;
};

UCLASS(meta = (PrioritizeCategories = "Item"))
class FORESTED_API AItemActorSpawner : public AActor {
	GENERATED_BODY()

public:

	AItemActorSpawner() {
		PrimaryActorTick.bCanEverTick = false;
	}

protected:

	virtual void BeginPlay() override {
		Super::BeginPlay();
		AItemActor::SpawnItemActor(GetWorld(), GetTransform(), ItemToSpawn);
	}

public:

	//the item to spawn, will use the actor class when spawed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayPriority = 0), Instanced, Category = "Item")
	UItem* ItemToSpawn = nullptr;
};