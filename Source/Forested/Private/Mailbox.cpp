#include "Mailbox.h"

#include "FPlayer.h"
#include "PlayerWidget.h"
#include "Components/BoxComponent.h"

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
	MailboxWidget = UPlayerWidget::CreatePlayerWidget<UMailboxWidget>(Player, WidgetClass);
	MailboxWidget->ActivateMailboxWidget(this);
	return true;
}

