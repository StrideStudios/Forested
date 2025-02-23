#include "Widget/ForestedUserWidget.h"

#if PLATFORM_DESKTOP || WITH_EDITOR
// This is only relevant on platforms with mouse support
void UForestedUserWidget::NativeOnFocusChanging(const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewWidgetPath, const FFocusEvent& InFocusEvent) {
	if (PreviousFocusPath.IsValid() && NewWidgetPath.IsValid()) {
		const FName WidgetName = NewWidgetPath.GetLastWidget().Get().GetType();
		if (InFocusEvent.GetCause() == EFocusCause::Mouse && WidgetName == FName("SViewport")) {
			if (bIsFocusable) SetFocus();
			return;
		}
	} 
	Super::NativeOnFocusChanging(PreviousFocusPath, NewWidgetPath, InFocusEvent);
}
#endif