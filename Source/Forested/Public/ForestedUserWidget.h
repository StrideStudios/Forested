#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ForestedUserWidget.generated.h"

UCLASS(meta = (PrioritizeCategories = "Widget"))
class FORESTED_API UForestedUserWidget : public UUserWidget {
	GENERATED_BODY()

protected:

#if PLATFORM_DESKTOP || WITH_EDITOR
	virtual void NativeOnFocusChanging(const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewWidgetPath, const FFocusEvent& InFocusEvent) override;
#endif

};