#pragma once

#include "DamageableInterface.h"
#include "TreeActor.h"
#include "Subsystems/WorldSubsystem.h"
#include "TreeSubsystem.generated.h"

#define TREE_SUBSYSTEM GetWorld()->GetSubsystem<UTreeSubsystem>()

class ATreeChildActor;
class ATreeActor;
class UHierarchicalInstancedStaticMeshComponent;

UCLASS(NotPlaceable, Abstract) //Hidden dont work lol
class FORESTED_API AInstanceActorBase : public AActor, public IDamageableInterface {
	GENERATED_BODY()
	
public:

	AInstanceActorBase();

	virtual void Init(UStaticMesh* Mesh);

	virtual void Damage_Implementation(AFPlayer* Player, const FHitResult& HitResult, float Damage, EDamageType DamageType) override;

	int32 AddActor(AActor* Actor);
	
	int32 RemoveActor(AActor* Actor);

	void UpdateActorTransform(const AActor* Actor, const FTransform& Transform);
	
	virtual void UpdateCustomData(int32 InstanceIndex, const AActor* Actor) const {}
	
	bool ContainsActor(const AActor* Actor) const {
		return Actors.Contains(Actor);
	}

protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHierarchicalInstancedStaticMeshComponent* InstancedMeshComponent;

	UPROPERTY()
	TMap<AActor*, int32> Actors;
};

UCLASS()
class FORESTED_API ATreeActorInstanceActor final : public AInstanceActorBase {
	GENERATED_BODY()
	
};

UCLASS()
class FORESTED_API ATreeChildInstanceActor final : public AInstanceActorBase {
	GENERATED_BODY()

public:
	
	void UpdateTree(const ATreeActor* TreeActor, const ATreeChildActor* TreeChildActor);
	
	virtual void UpdateCustomData(int32 InstanceIndex, const AActor* Actor) const override;
	
};

UCLASS(BlueprintType, Transient)
class FORESTED_API UTreeSubsystem : public UWorldSubsystem {
	GENERATED_BODY()

public:
	//UWorldSubsystem implementation
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

	//USubsystem Implementation
	virtual void Initialize(FSubsystemCollectionBase& Collection) override {}
	virtual void Deinitialize() override {}

	ATreeActorInstanceActor* GetOrCreateTreeActorInstance(UStaticMesh* Mesh) {
		return CastChecked<ATreeActorInstanceActor>(GetTreeInstanceActor_Internal(Mesh, ATreeActorInstanceActor::StaticClass()));
	}
	
	ATreeChildInstanceActor* GetOrCreateTreeChildInstance(UStaticMesh* Mesh) {
		return CastChecked<ATreeChildInstanceActor>(GetTreeInstanceActor_Internal(Mesh, ATreeChildInstanceActor::StaticClass()));
	}
	
	void AddTree(ATreeActor* TreeActor);
	
	void RemoveTree(ATreeActor* TreeActor);

	bool HasTreeInstance(const ATreeActor* TreeActor) const;

	void AddTreeChild(ATreeActor* TreeActor, const ATreeChildActor* TreeChildActor);

	void RemoveTreeChild(ATreeActor* TreeActor, const ATreeChildActor* TreeChildActor);
	

	bool HasTreeChildInstance(const ATreeActor* TreeActor, const ATreeChildActor* TreeChildActor) const;

	void SetInstanceTransform(const ATreeActor* TreeActor, const ATreeChildActor* TreeChildActor);

	void ResetInstanceTransform(const ATreeActor* TreeActor, const ATreeChildActor* TreeChildActor);

	UPROPERTY()
	TMap<UStaticMesh*, AInstanceActorBase*> TreeInstanceActors;

private:

	AInstanceActorBase* GetTreeInstanceActor_Internal(UStaticMesh* Mesh, TSubclassOf<AInstanceActorBase> Class);

};