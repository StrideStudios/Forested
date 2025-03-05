#pragma once

#include "Forested/ForestedMinimal.h"
#include "Widget/PlayerWidget.h"
#include "Interfaces/SelectableInterface.h"
#include "GameFramework/Actor.h"
#include "Mailbox.generated.h"

class AMailbox;
class UBoxComponent;

UCLASS(Blueprintable, meta=(PrioritizeCategories = "Widget Components"))
class FORESTED_API UMailboxWidget : public UPlayerWidget {
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent)
	void ActivateMailboxWidget(AMailbox* Mailbox);

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