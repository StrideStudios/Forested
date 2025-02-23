#include "Workbench.h"
#include "Player/FPlayer.h"
#include "Components/BoxComponent.h"
#include "Player/PlayerInventory.h"
#include "Serialization/SerializationLibrary.h"

AWorkbenchRenderActor::AWorkbenchRenderActor() {
	PrimaryActorTick.bCanEverTick = false;

    BoundsBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Bounds Box"));
    BoundsBox->SetupAttachment(GetRootComponent());
	BoundsBox->SetGenerateOverlapEvents(false);
	BoundsBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoundsBox->SetHiddenInGame(false);
	SelectionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Selection Box"));
	SelectionBox->SetupAttachment(GetRootComponent());
	SelectionBox->SetGenerateOverlapEvents(true);
	SelectionBox->SetCollisionResponseToAllChannels(ECR_Overlap);
	SelectionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	SelectionBox->SetHiddenInGame(false);
}

AWorkbench::AWorkbench(): ASelectableInventoryActor("Workbench", 2) {
	PrimaryActorTick.bCanEverTick = false;
	
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	ItemOneComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("Item One Component"));
	ItemOneComponent->SetupAttachment(GetRootComponent());
	ItemTwoComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("Item Two Component"));
	ItemTwoComponent->SetupAttachment(GetRootComponent());
	WorkbenchBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Workbench Box"));
	WorkbenchBox->SetupAttachment(GetRootComponent());
	HammerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Hammer Box"));
	HammerBox->SetupAttachment(GetRootComponent());
}

void AWorkbench::BeginPlay() {
	Super::BeginPlay();
}

bool AWorkbench::Selected_Implementation(AFPlayer* Player, const FHitResult& HitResult, const float HeldTime) {
	if (ItemOneComponent->GetChildActor() && HitResult.GetActor() == ItemOneComponent->GetChildActor()) {
		return Player->PlayerInventory->SwapItemFrom(this, 0);
	}
	if (ItemTwoComponent->GetChildActor() && HitResult.GetActor() == ItemTwoComponent->GetChildActor()) {
		return Player->PlayerInventory->SwapItemFrom(this, 1);
	}
	FItemHeap SelectedItem;
	if (!Player->PlayerInventory->GetSelectedItem(SelectedItem) || !SelectedItem->GetClass()->ImplementsInterface(UWorkbenchInterface::StaticClass())) return false;
	return SwapItemFrom(Player->PlayerInventory, Player->PlayerInventory->GetSelectedSlot());
}

void AWorkbench::OnInsertItem(const FItemHeap Item, const int Slot) {
	RegisterItemComponent(Slot);
	Super::OnInsertItem(Item, Slot);
}

void AWorkbench::OnRemoveItem(const FItemHeap Item, const int Slot) {
	ClearItemComponent(Slot);
	Super::OnRemoveItem(Item, Slot);
}


void AWorkbench::ClearItemComponent(const int Slot) {
	UChildActorComponent*& Component = Slot == 0 ? ItemOneComponent : ItemTwoComponent;
	Component->SetChildActorClass(nullptr);
}

void AWorkbench::RegisterItemComponent(const int Slot) {
	FItemHeap Item;
	if (GetItem(Slot, Item)) {
		TSoftClassPtr<AWorkbenchRenderActor> RenderActor = IWorkbenchInterface::Execute_GetWorkbenchRenderActor(Item);
		FSerializationLibrary::LoadAsync(RenderActor, [this, Slot, RenderActor] {
			UChildActorComponent*& Component = Slot == 0 ? ItemOneComponent : ItemTwoComponent;
			Component->SetChildActorClass(RenderActor.Get());
			if (AWorkbenchRenderActor* WorkbenchRenderActor = Cast<AWorkbenchRenderActor>(Component->GetChildActor())) {
				WorkbenchRenderActor->Init();
			}
		});
	}
}

