#pragma once

#include "Forested/ForestedMinimal.h"
#include "Widget/PlayerWidget.h"
#include "Interfaces/SelectableInterface.h"
#include "GameFramework/Actor.h"
#include "Mailbox.generated.h"

class AMailbox;
class UBoxComponent;

UCLASS(Blueprintable)
class FORESTED_API UMailboxWidget : public UPlayerWidget {
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Mailbox Widget")
	void ActivateMailboxWidget(AMailbox* InMailbox);

	UFUNCTION(BlueprintCallable, Category = "Mailbox Widget")
	void CloseMailboxMenu();

protected:
	
	UPROPERTY(BlueprintReadOnly, Category = "Mailbox Widget")
	AMailbox* Mailbox;
};

UCLASS(Blueprintable, meta=(PrioritizeCategories = "Widget Components"))
class FORESTED_API AMailbox : public AActor, public ISelectableInterface {
	GENERATED_BODY()
	
public:	
	AMailbox();

protected:
	virtual void BeginPlay() override;

public:

	virtual bool Selected_Implementation(AFPlayer* Player, const FHitResult& HitResult, float HeldTime) override;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Mailbox")
	void OpenMailbox();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Mailbox")
	void CloseMailbox();
	
protected:

	UPROPERTY(BlueprintReadOnly, Category = "Widget")
	UMailboxWidget* MailboxWidget;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, NoClear, Category = "Widget")
	TSoftClassPtr<UMailboxWidget> WidgetClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* BoxCollisionComponent;
};