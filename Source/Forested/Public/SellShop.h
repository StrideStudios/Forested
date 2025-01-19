#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item/Item.h"
#include "SellShop.generated.h"

class UBoxComponent;

UCLASS()
class FORESTED_API ASellShop : public AActor {
	GENERATED_BODY()
	
public:	
	ASellShop();

	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	USceneComponent* Root;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	UBoxComponent* BoxCollision;

protected:

	UFUNCTION()
	void OnNewDay(int Day);

};

UINTERFACE()
class USellInterface : public UInterface {
	GENERATED_BODY()
};

class ISellInterface {
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent)
	void OnItemSold(ASellShop* Shop);
	virtual void OnItemSold_Implementation(ASellShop* Shop) {}

	UFUNCTION(BlueprintNativeEvent, Category = "Sell Interface")
	int GetWorth();
	virtual int GetWorth_Implementation() {
		return 0;
	}
	
};