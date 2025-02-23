#pragma once

#include "Forested/ForestedMinimal.h"
#include "GameFramework/Actor.h"
#include "RenderActor.generated.h"

/*
 * A class designed to render an item in the scene by having static mesh components
 * Used in rendering items in item actors, workbench, player hands, etc.
 */
UCLASS(meta = (PrioritizeCategories = "Render"))
class FORESTED_API ARenderActor : public AActor  {
	GENERATED_BODY()
	
public:	
	ARenderActor():
	ARenderActor(true) {
	}
	
	ARenderActor(bool EnableCull);

	UFUNCTION(BlueprintCallable, Category = "Render Actor")
	virtual void Init();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, DisplayName = "Init", Category = "Render Actor")
	void ReceiveInit();

	UFUNCTION(BlueprintCallable, Category = "Render Actor")
	void SetSimulatePhysics(bool SimulatePhysics);

	UFUNCTION(BlueprintCallable, Category = "Item Actor")
	virtual void RefreshMeshBounds(float CullDistanceOverride = 0.f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USceneComponent* Root;
	
protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Render")
	FName AttachSocket = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Render")
	bool EnableCull;

	void SetupRootAttachment() const;
	
	UPROPERTY()
	TArray<UPrimitiveComponent*> PrimitiveComponents;

};