#pragma once

#include "CoreMinimal.h"
#include "SelectableInterface.h"
#include "Item/Item.h"
#include "Item/InventoryActor.h"
#include "Item/RenderActor.h"
#include "Workbench.generated.h"

class AWorkbench;
class UBoxComponent;

USTRUCT(BlueprintType)
struct FORESTED_API FWorkbenchRecipe {
	GENERATED_BODY()
	
	FWorkbenchRecipe():
	FWorkbenchRecipe({}, {}, {}){ 
	}

	FWorkbenchRecipe(FItemHeap FirstIngredient, FItemHeap SecondIngredient, FItemHeap Result):
	FirstIngredient(FirstIngredient),
	SecondIngredient(SecondIngredient),
	Result(Result) {
	}

	operator bool() const {
		return FirstIngredient && SecondIngredient && Result;
	}

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FItemHeap FirstIngredient;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FItemHeap SecondIngredient;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FItemHeap Result;
	
};

/*
 *	Used to define logic for rendering an item in workbench
 */
UCLASS()
class FORESTED_API AWorkbenchRenderActor : public ARenderActor {
	GENERATED_BODY()

public:
	
	AWorkbenchRenderActor();

	UFUNCTION(BlueprintImplementableEvent, Category = "Workbench Render Actor")
	void SelectActor(AWorkbench* Workbench);
	
	//this box is used to define the bounds for sitting on the workbench, including how to sit on top of the workbench
	UPROPERTY(BlueprintReadOnly, Category = "Mesh")
    UBoxComponent* BoundsBox;

	//this box is used to define how to select the object on the workbench (generally keep limited to component size)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UBoxComponent* SelectionBox;
};

/*
 * Contains the data used for an object in workbench, properties are assumed non-null
 */
UINTERFACE()
class UWorkbenchInterface : public UInterface {
	GENERATED_BODY()
};

class IWorkbenchInterface {
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, Category = "Workbench Interface")
	TSoftClassPtr<AWorkbenchRenderActor> GetWorkbenchRenderActor();
	virtual TSoftClassPtr<AWorkbenchRenderActor> GetWorkbenchRenderActor_Implementation() {
		return nullptr;
	}
	
};

UCLASS()
class FORESTED_API AWorkbench : public ASelectableInventoryActor {
	GENERATED_BODY()
	
public:
	
	AWorkbench();
	
	virtual void BeginPlay() override;

	virtual bool Selected_Implementation(AFPlayer* Player, const FHitResult& HitResult, float HeldTime) override;

	virtual void OnGameLoad_Implementation(UObject* WorldContextObject, UObjectSaveGame* SaveGame) override {
		Super::OnGameLoad_Implementation(WorldContextObject, SaveGame);
		RegisterItemComponents();
	}
	
	virtual void OnInsertItem(FItemHeap Item, const int Slot) override;

	virtual void OnRemoveItem(FItemHeap Item, const int Slot) override;

	void ClearItemComponent(int Slot);
	
	void RegisterItemComponent(int Slot);

	void RegisterItemComponents() {
		RegisterItemComponent(0);
		RegisterItemComponent(1);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Workbench")
	bool GetItemOne(FItemHeap& OutItem) const { return GetItem(0, OutItem); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Workbench")
	bool GetItemTwo(FItemHeap& OutItem) const { return GetItem(1, OutItem); }
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UBoxComponent* WorkbenchBox;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UBoxComponent* HammerBox;


	//have two components in workbench tag to identify which goes where (some sort of box to indicate boundries)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UChildActorComponent* ItemOneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UChildActorComponent* ItemTwoComponent;
	
};