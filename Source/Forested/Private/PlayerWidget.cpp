#include "PlayerWidget.h"
#include "FPlayer.h"
#include "LevelDefaults.h"
#include "PlayerInputComponent.h"
#include "SelectableInterface.h"
#include "Animation/UMGSequencePlayer.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/WidgetSwitcher.h"

//TODO: support for child widget

bool UPlayerWidget::Initialize() {

	const bool Init = Super::Initialize();

	const UPlayerWidget* OwningPlayerWidget = GetTypedOuter<UPlayerWidget>();
	if (OwningPlayerWidget && Init) {
		SetFocus = false; //up to the parent to set focus and add to viewport
	}
	
	return Init;
}

void UPlayerWidget::RemoveWidget() {
	if (SetFocus) {
		GetWidgetPlayer()->SetGameFocus();
	}
	RemoveFromParent();
}

AFPlayer* UPlayerWidget::GetWidgetPlayer() const {
	if (const APlayerController* PlayerController = GetOwningPlayer()) {
		return CastChecked<AFPlayer>(PlayerController->GetPawn());
	}
	return nullptr;
}
//TODO: creating widgets removes focus?
UPlayerWidget* UPlayerWidget::CreatePlayerWidget_Internal(const AFPlayer* InPlayer, const TSubclassOf<UPlayerWidget>& WidgetClass, const bool InSetFocus) {
	APlayerController* PlayerController = CastChecked<APlayerController>(InPlayer->GetController());
	UPlayerWidget* Widget = CreateWidget<UPlayerWidget>(PlayerController, WidgetClass);
	Widget->AddToViewport();//exception, probably widgetclass
	Widget->SetFocus = InSetFocus;
	if (InSetFocus) {
		InPlayer->SetUIFocus(Widget);
	}
	return Widget;
}

void UPlayerHud::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime) {
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

FReply UPlayerHud::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) {
	if (InKeyEvent.GetKey() == EKeys::Escape && IsInMenu()) {
		SetGameHud();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UPlayerHud::RunInventoryAnimation() {
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	if (!Timer.IsValid()) {
		PlayAnimationForward(GetInventoryAnimation());
	}
	TimerManager.SetTimer(Timer, [this] {
		PlayAnimationReverse(GetInventoryAnimation());
		Timer.Invalidate();
	}, 5.f, false);
}

void UPlayerHud::SetWidgetVisible() {
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

void UPlayerHud::SetWidgetCollapsed() {
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
