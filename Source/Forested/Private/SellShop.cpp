#include "SellShop.h"
#include "Kismet/GameplayStatics.h"
#include "Player/FPlayer.h"
#include "Components/BoxComponent.h"
#include "Sky.h"
#include "Items/Item.h"
#include "Items/ItemActor.h"

ASellShop::ASellShop() {
	PrimaryActorTick.bCanEverTick = false;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collision"));
	BoxCollision->SetupAttachment(GetRootComponent());
	BoxCollision->SetCollisionProfileName(FName(TEXT("OverlapAllDynamic")));
}

void ASellShop::BeginPlay() {
	Super::BeginPlay();
	if (SKY) {
		SKY->OnNewDay.AddUniqueDynamic(this, &ASellShop::OnNewDay);
	} else {
		LOG_ERROR("Error; Sky Not Found; A Sky Must Be In Your Scene");
	}
}

void ASellShop::OnNewDay(int Day) {
	if (PLAYER) {
		TArray<AActor*> ActorArray;
		BoxCollision->GetOverlappingActors(ActorArray);
		for (AActor* Actor : ActorArray) {
			if (AItemActor* ItemActor = Cast<AItemActor, AActor>(Actor)) {
				if (ItemActor->GetItem()->GetClass()->ImplementsInterface(USellInterface::StaticClass())) {
					const int ItemWorth = ISellInterface::Execute_GetWorth(ItemActor->GetItem());
					ISellInterface::Execute_OnItemSold(ItemActor->GetItem(), this);
					PLAYER->AddMoney(ItemWorth);
					ItemActor->Destroy();
				}
			}
		}
	}
}
