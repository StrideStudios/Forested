#pragma once

#include "CoreMinimal.h"
#include "DamageableInterface.h"
#include "Item/InventoryActor.h"
#include "ChopStump.generated.h"

UENUM(BlueprintType)
enum class ETreeLogType : uint8 {
	//no log is present
	None UMETA(DisplayName = "None"),
	//the full unsplit log is present
	Full UMETA(DisplayName = "Full"),
	//the log has been split in half
	Half UMETA(DisplayName = "Half"),
	//the log has been split into quarters
	Quarter UMETA(DisplayName = "Quarter")
};

UCLASS()
class AChopStump : public AOneSelectableInventoryActor, public IDamageableInterface {
	GENERATED_BODY()

public:

	AChopStump();

	virtual bool CanItemBeInserted(FItemHeap Item, const int Slot) override;
	
};

UINTERFACE()
class UChopStumpInterface : public UInterface {
	GENERATED_BODY()
};

class IChopStumpInterface {
	GENERATED_BODY()

public:

	/**
	 * Event that fires when an item is put on the stump
	 * @param ChopStump The stump it was put on
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Chop Stump Interface")
	void OnAddedToChopStump(AChopStump* ChopStump);
	virtual void OnAddedToChopStump_Implementation(AChopStump* ChopStump) {}

	/**
	 * Get Properties for the chop stump
	 * @param TreeLogType The type of log being put on the stump
	 * @param StaticMesh the mesh of the log
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Chop Stump Interface")
	void GetChopStumpProperties(ETreeLogType& TreeLogType, TSoftObjectPtr<UStaticMesh>& StaticMesh) const;
	virtual void GetChopStumpProperties_Implementation(ETreeLogType& TreeLogType, TSoftObjectPtr<UStaticMesh>& StaticMesh) const {
		TreeLogType = ETreeLogType::None;
		StaticMesh = nullptr;
	}

};