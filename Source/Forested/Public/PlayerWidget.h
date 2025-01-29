#pragma once

#include "CoreMinimal.h"
#include "ForestedUserWidget.h"
#include "PlayerWidget.generated.h"

class UImage;
class UWidgetSwitcher;
class AFPlayer;
class UBoxComponent;

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

	virtual bool Initialize() override;

	template<class WidgetType = UPlayerWidget>
	static WidgetType* CreatePlayerWidget(AFPlayer* InPlayer, const TSubclassOf<UPlayerWidget>& WidgetClass = WidgetType::StaticClass(), const bool InSetFocus = true) {
		if (!WidgetClass) return nullptr;
		return static_cast<WidgetType*>(CreatePlayerWidget_Internal(InPlayer, WidgetClass, InSetFocus));
	}
	
	UFUNCTION(BlueprintCallable, Category = "Widget")
	void RemoveWidget();

protected:

	UFUNCTION(BlueprintCallable, Category = "Widget")
	AFPlayer* GetWidgetPlayer() const;

private:

	UFUNCTION(BlueprintCallable, DisplayName = "Create Player Widget", meta = (DeterminesOutputType="WidgetClass", AdvancedDisplay = 2), Category = "Widget")
	static UPlayerWidget* ReceiveCreatePlayerWidget(AFPlayer* InPlayer, const TSubclassOf<UPlayerWidget> WidgetClass, const bool InSetFocus = true) {
		if (!WidgetClass) return nullptr;
		return CreatePlayerWidget_Internal(InPlayer, WidgetClass, InSetFocus);
	}
	
	static UPlayerWidget* CreatePlayerWidget_Internal(const AFPlayer* InPlayer, const TSubclassOf<UPlayerWidget>& WidgetClass, bool InSetFocus);

	bool SetFocus;
	
};

UCLASS(Blueprintable)
class FORESTED_API UPlayerHud : public UPlayerWidget {
	GENERATED_BODY()

	friend class AFPlayer;

public:

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	FORCEINLINE bool IsInMenu() const { return GetCurrentWidgetIndex() == 0; }
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Player Hud")
	void SetSlotSelected(int InventorySlot, bool Selected);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Player Hud")
	void UpdateSlot(int InventorySlot);

protected:

	UFUNCTION(BlueprintCallable, Category = "Widget")
	void RunInventoryAnimation();

	UFUNCTION(BlueprintImplementableEvent, Category = "Widget")
	UWidgetAnimation* GetInventoryAnimation();

	UFUNCTION(BlueprintImplementableEvent, Category = "Widget")
	UWidgetAnimation* GetHideHudAnimation();
	
	UFUNCTION(BlueprintCallable, Category = "Widget")
	virtual void SetWidgetVisible();
	
	UFUNCTION(BlueprintCallable, Category = "Widget")
	virtual void SetWidgetCollapsed();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Widget")
	int GetCurrentWidgetIndex() const;
	
	UFUNCTION(BlueprintCallable, Category = "Widget")
	void SetMenuHud();
	
	UFUNCTION(BlueprintCallable, Category = "Widget")
	void SetGameHud();

	UFUNCTION(BlueprintImplementableEvent, Category = "Widget")
	void OnSetMenuHud();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Widget")
	void OnSetGameHud();
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UWidgetSwitcher* WidgetSwitcher;
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* CenterImage;

private:

	UPROPERTY()
	UUMGSequencePlayer* HideHudPlayer;
	
	FTimerHandle Timer;
	
};