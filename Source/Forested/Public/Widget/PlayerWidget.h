#pragma once

#include "Forested/ForestedMinimal.h"
#include "ForestedUserWidget.h"
#include "PlayerWidget.generated.h"

class AFPlayer;

/*
 * Proper Parent Components
UWidget
UWidgetTree
APlayerController
UGameInstance
UWorld
 */

UCLASS(Blueprintable, meta=(PrioritizeCategories = "Widget"))
class FORESTED_API UPlayerWidget : public UForestedUserWidget {
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = "Widget")
	AFPlayer* GetWidgetPlayer() const;
	
};
