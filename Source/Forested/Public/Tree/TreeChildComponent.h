#pragma once

#include "CoreMinimal.h"
#include "CullStaticMeshComponent.h"
#include "Forested/Forested.h"
#include "Item/Item.h"
#include "Item/ItemActor.h"
#include "Item/PlayerInventory.h"
#include "TreeChildComponent.generated.h"

class AItemActor;
class UNiagaraSystem;

UCLASS()
class FORESTED_API UTreeProductItem : public UItem, public IItemActorInterface {
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Tree Item")
	float CurrentGrowth = 0.f;

};

UCLASS()
class FORESTED_API UTreeItem : public UItem, public IItemActorInterface {
	GENERATED_BODY()

public:

	virtual void OnAddedToInventory_Implementation(const TScriptInterface<IInventoryBase>& Inventory, int Slot) override {
		HasBeenPickedUp = true;
	}

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Tree Item")
	bool HasBeenPickedUp = false;

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Tree Item")
	int SavedTreeIndex = 0;

};

UCLASS(Abstract, meta = (PrioritizeCategories = "Type"))
class FORESTED_API UTreeChildComponent : public UCullStaticMeshComponent {
	GENERATED_BODY()

public:

	UTreeChildComponent() {
		SetHiddenInGame(true);
		SetGenerateOverlapEvents(false);
		UPrimitiveComponent::SetCollisionEnabled(ECollisionEnabled::NoCollision);
		UPrimitiveComponent::SetCollisionResponseToChannel(LANDSCAPE_OBJECT_CHANNEL, ECR_Overlap);
		USceneComponent::SetMobility(EComponentMobility::Static);
	}
	
protected:

	virtual void BeginPlay() override;
	
public:
	
	template<class UserClass = UTreeChildComponent>
	void GetTreeChildComponents(TArray<UserClass*>& TreeComponents) {
		TreeComponents.Add(CastChecked<UserClass>(this));
		TArray<USceneComponent*> Components;
		GetChildrenComponents(false, Components);
		Components.Sort([](const USceneComponent& S1, const USceneComponent& S2) {
			return S1.GetName() > S2.GetName();
		});
		for (USceneComponent* Component : Components) {
			if (!Component) continue;
			if (UserClass* TreeComponent = Cast<UserClass>(Component)) {
				TreeComponent->GetTreeChildComponents(TreeComponents);
			}
		}
	}

	virtual void Init() {
	}

	virtual void EnableCollision() {
		USceneComponent::SetMobility(EComponentMobility::Movable);
		SetHiddenInGame(false);
	}
	
	virtual bool SpawnItemActor() {
		return false;
	}

};

UCLASS(Blueprintable, ClassGroup = (Rendering, Common), editinlinenew, meta = (BlueprintSpawnableComponent))
class UTrunkComponent : public UTreeChildComponent {
	GENERATED_BODY()

	friend class ATreeSplitActor;
	
public:

	virtual void EnableCollision() override {
		Super::EnableCollision();
		SetGenerateOverlapEvents(true);
		UPrimitiveComponent::SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	
	void Init(UTreeItem* InItem, const TSoftObjectPtr<UStaticMesh>& InStaticMesh);

	virtual void Init() override {
		FString NumberString;
		for (const char c : GetName()) {
			if (c >= '0' && c <= '9') {
				NumberString += c;
			}
		}
		if (!NumberString.IsNumeric()) {
			Item->SavedTreeIndex = 0;
			return;
		}
		uint32 Num;
		TTypeFromString<uint32>::FromString(Num, *NumberString);
		Item->SavedTreeIndex = Num;
	}
	
	virtual bool SpawnItemActor() override;

private:
	
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Type")
	UTreeItem* Item = nullptr;

};

UCLASS(Blueprintable, ClassGroup = (Rendering, Common), editinlinenew, meta = (BlueprintSpawnableComponent))
class UBranchComponent : public UTreeChildComponent {
	GENERATED_BODY()

public:

	virtual void EnableCollision() override {
		Super::EnableCollision();
		SetGenerateOverlapEvents(true);
		UPrimitiveComponent::SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	virtual bool SpawnItemActor() override;

	virtual void Init() override {
		FString NumberString;
		for (const char c : GetName()) {
			if (c >= '0' && c <= '9') {
				NumberString += c;
			}
		}
		if (!NumberString.IsNumeric()) {
			Item->SavedTreeIndex = 0;
			return;
		}
		uint32 Num;
		TTypeFromString<uint32>::FromString(Num, *NumberString);
		Item->SavedTreeIndex = Num;
	}

private:

	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Type")
	UTreeItem* Item = nullptr;
	
};

UCLASS(Blueprintable, ClassGroup = (Rendering, Common), editinlinenew, meta = (BlueprintSpawnableComponent, PrioritizeCategories = "Growth"))
class UProductComponent : public UTreeChildComponent {
	GENERATED_BODY()

public:

	virtual void Init() override {
	}
	
	void Init(const float InStartGrowthTime, const float InEndGrowthTime) {
		StartGrowthTime = InStartGrowthTime;
		EndGrowthTime = InEndGrowthTime;
	}

	virtual bool SpawnItemActor() override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tree Child Component")
	float GetCurrentSize() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tree Child Component")
	float GetCurrentGrowth() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tree Child Component")
	FORCEINLINE float GetDaysUntilGrown(const float Random = 0.f) const {
		return DaysUntilGrown + GrowthRandom * (Random * 2.f - 1.f);
	}

	UPROPERTY(EditDefaultsOnly, Category = "Growth", meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
	float RequiredGrowth = 75.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Growth", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DaysUntilGrown = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Growth", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float GrowthRandom = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Growth")
	UNiagaraSystem* FallNiagaraSystem = nullptr;

private:

	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Type")
	UTreeProductItem* Item = nullptr;

	UPROPERTY(SaveGame)
	float StartGrowthTime = 0.f;

	UPROPERTY(SaveGame)
	float EndGrowthTime = 0.f;
};

UCLASS(NotPlaceable)
class FORESTED_API ATreeItemActor : public AItemActor {
	GENERATED_BODY()

	virtual void Init() override;
};