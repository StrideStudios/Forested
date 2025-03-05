#pragma once

#include "Forested/ForestedMinimal.h"
#include "Widget/PlayerWidget.h"
#include "PlayerHud.generated.h"

class UWidgetSwitcher;
class UImage;

UCLASS(Blueprintable)
class FORESTED_API UInventoryWidget : public UPlayerWidget {
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Player Hud")
	void SetSlotSelected(int InventorySlot, bool Selected);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Player Hud")
	void UpdateSlot(int InventorySlot);
	
protected:

	UFUNCTION(BlueprintCallable, Category = "Widget")
	void RunInventoryAnimation();

	UFUNCTION(BlueprintImplementableEvent, Category = "Widget")
	UWidgetAnimation* GetInventoryAnimation();
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* CenterImage;

private:

	UPROPERTY()
	UUMGSequencePlayer* HideHudPlayer;

	FTimerHandle Timer;
	
};

UCLASS(Blueprintable)
class FORESTED_API UMenuWidget : public UPlayerWidget {
	GENERATED_BODY()

public:
	
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	
};

UCLASS(Blueprintable)
class FORESTED_API UPlayerHud : public UPlayerWidget {
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;

	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Widget")
	FORCEINLINE UInventoryWidget* GetInventoryWidget() const { return InventoryWidget; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Widget")
	FORCEINLINE UMenuWidget* GetMenuWidget() const { return MenuWidget; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Widget")
	FORCEINLINE bool IsInMenu() const { return GetCurrentWidgetIndex() == 0; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Widget")
	int GetCurrentWidgetIndex() const;
	
	UFUNCTION(BlueprintCallable, Category = "Widget")
	void ShowHud();
	
	UFUNCTION(BlueprintCallable, Category = "Widget")
	void HideHud();

	UFUNCTION(BlueprintCallable, Category = "Widget")
	void SetMenuHud();
	
	UFUNCTION(BlueprintCallable, Category = "Widget")
	void SetGameHud();

protected:
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Widget")
	UWidgetAnimation* GetHideHudAnimation();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Widget")
	void OnSetMenuHud();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Widget")
	void OnSetGameHud();
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UWidgetSwitcher* WidgetSwitcher;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UInventoryWidget* InventoryWidget;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UMenuWidget* MenuWidget;

private:

	UPROPERTY()
	UUMGSequencePlayer* HideHudPlayer;
	
};