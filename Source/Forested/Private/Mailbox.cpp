#include "Mailbox.h"
#include "Player/FPlayer.h"
#include "Widget/PlayerWidget.h"
#include "Components/BoxComponent.h"
#include "Player/PlayerHud.h"
#include "Player/PlayerInventory.h"
#include "Serialization/SerializationLibrary.h"

void UMailboxWidget::ActivateMailboxWidget(AMailbox* InMailbox) {
	Mailbox = InMailbox;
}

void UMailboxWidget::CloseMailboxMenu() {
	RemoveFromParent();
	Mailbox->CloseMailbox();
}

AMailbox::AMailbox() {
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	WidgetClass = UMailboxWidget::StaticClass();

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh Component"));
	RootComponent = StaticMeshComponent;
	BoxCollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collision Component"));
	BoxCollisionComponent->SetupAttachment(StaticMeshComponent);
}

void AMailbox::BeginPlay() {
	Super::BeginPlay();
}

bool AMailbox::Selected_Implementation(AFPlayer* Player, const FHitResult& HitResult, const float HeldTime) {
	if (WidgetClass.IsNull()) return false;

	if (!Player->PlayerInventory->ClearSelectedItem(2.f)) {
		return false;
	}
	
	FSerializationLibrary::LoadSync(WidgetClass);
	MailboxWidget = CreateWidget<UMailboxWidget>(Player->GetPlayerController(), WidgetClass.Get());
	MailboxWidget->AddToViewport(1);
	MailboxWidget->ActivateMailboxWidget(this);

	Player->SetUIFocus(MailboxWidget);
	Player->GetHud()->HideHud();
	OpenMailbox();
	
	return true;
}

