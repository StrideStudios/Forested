#include "Widget/PlayerWidget.h"
#include "Player/FPlayer.h"
#include "Player/PlayerInputComponent.h"
#include "Interfaces/SelectableInterface.h"
#include "Animation/UMGSequencePlayer.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Player/PlayerInventory.h"
#include "Serialization/SerializationLibrary.h"

AFPlayer* UPlayerWidget::GetWidgetPlayer() const {
	if (const APlayerController* PlayerController = GetOwningPlayer()) {
		return CastChecked<AFPlayer>(PlayerController->GetPawn());
	}
	return nullptr;
}

/*
void UPlayerHud::SetMenuHud() {
	WidgetSwitcher->SetActiveWidgetIndex(0);
	GetWidgetPlayer()->SetUIFocus(this);
	SetVisibility(ESlateVisibility::Visible);
	OnSetMenuHud();
}

void UPlayerHud::SetGameHud() {
	WidgetSwitcher->SetActiveWidgetIndex(1);
	GetWidgetPlayer()->SetGameFocus();
	SetVisibility(ESlateVisibility::HitTestInvisible);
	OnSetGameHud();
}
*/