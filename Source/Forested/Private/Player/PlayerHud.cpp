#include "Player/PlayerHud.h"
#include "Animation/UMGSequencePlayer.h"
#include "Components/Image.h"
#include "Components/WidgetSwitcher.h"
#include "Interfaces/SelectableInterface.h"
#include "Player/FPlayer.h"
#include "Player/PlayerInventory.h"
#include "Serialization/SerializationLibrary.h"
#include "Widget/PlayerWidget.h"

void UInventoryWidget::NativeConstruct() {
	const UPlayerInventory* PlayerInventory = GetWidgetPlayer()->PlayerInventory;
	for (int i = 0; i < PlayerInventory->GetCapacity(); ++i) {
		UpdateSlot(i);
	}
	SetSlotSelected(PlayerInventory->GetSelectedSlot(), true);
}


void UInventoryWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime) {
	Super::NativeTick(MyGeometry, InDeltaTime);

	float TargetIconOpacity = 0.f;
	const AFPlayer* Player = GetWidgetPlayer();
	if (Player && Player->IsInputAllowed()) {
		const FHitResult HitResult = Player->GetHoveredHitResult();
		if (HitResult.bBlockingHit && HitResult.GetActor()) {
			const AActor* Actor = HitResult.GetActor()->IsChildActor() ? HitResult.GetActor()->GetParentActor() : HitResult.GetActor();
			if (Actor && Actor->GetClass()->ImplementsInterface(USelectableInterface::StaticClass())) {
				if (ISelectableInterface::Execute_CanSelect(Actor, Player, HitResult, 0.f)) {
					TargetIconOpacity = 1.f;
				}
			}
		}
	}

	const float IconOpacity = FMath::FInterpTo(CenterImage->GetRenderOpacity(), TargetIconOpacity, InDeltaTime, 5.f);
	CenterImage->SetRenderOpacity(IconOpacity);
}

void UInventoryWidget::RunInventoryAnimation() {
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	if (!Timer.IsValid()) {
		PlayAnimationForward(GetInventoryAnimation());
	}
	TimerManager.SetTimer(Timer, [this] {
		PlayAnimationReverse(GetInventoryAnimation());
		Timer.Invalidate();
	}, 5.f, false);
}

FReply UMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) {
	if (InKeyEvent.GetKey() == EKeys::Escape) {
		FORESTED_GAME_MODE->ResumeGame();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UPlayerHud::ShowHud() {
	if (!GetHideHudAnimation()) return;
	if (HideHudPlayer) {
		HideHudPlayer->Reverse();
	}
	
	HideHudPlayer = PlayAnimationReverse(GetHideHudAnimation());
	if (!HideHudPlayer) return;
	HideHudPlayer->OnSequenceFinishedPlaying().AddLambda([this](UUMGSequencePlayer& UmgSequencePlayer){
		HideHudPlayer = nullptr;
	});
}

void UPlayerHud::HideHud() {
	if (!GetHideHudAnimation()) return;
	if (HideHudPlayer) {
		HideHudPlayer->Reverse();
	}
	
	HideHudPlayer = PlayAnimationForward(GetHideHudAnimation());
	if (!HideHudPlayer) return;
	HideHudPlayer->OnSequenceFinishedPlaying().AddLambda([this](UUMGSequencePlayer& UmgSequencePlayer) {
		HideHudPlayer = nullptr;
	});
}

int UPlayerHud::GetCurrentWidgetIndex() const {
	return WidgetSwitcher->GetActiveWidgetIndex();
}

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