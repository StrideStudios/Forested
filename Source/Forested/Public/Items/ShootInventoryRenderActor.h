#pragma once

#include "Forested/ForestedMinimal.h"
#include "Player/PlayerInventoryActor.h"
#include "ShootInventoryRenderActor.generated.h"

UCLASS(BlueprintType, meta = (PrioritizeCategories = "Damage"))
class FORESTED_API AShootInventoryRenderActor : public APlayerInventoryActor {
	GENERATED_BODY()

public:
	
	AShootInventoryRenderActor();

	void Shoot();
};
