#include "FireActor.h"
#include "Player/FPlayer.h"
#include "Player/PlayerInventory.h"

AFireActor::AFireActor() {
}

bool AFireActor::Selected_Implementation(AFPlayer* Player, const FHitResult& HitResult, const float HeldTime) {
	FItemHeap Item;
	if (!Player->PlayerInventory->GetSelectedItem(Item)) return false;
	const bool bIsFuel = Item->GetClass()->ImplementsInterface(UFireFuelInterface::StaticClass());
	const bool bIsLighter = Item->GetClass()->ImplementsInterface(ULightFireInterface::StaticClass());
	if (!bIsFuel && !bIsLighter) {
		LOG_EDITOR_WARNING("Item %s could not be used on fire; It does not implement the fire fuel interface or the light fire interface", *Item->GetItemName().ToString());
		return false;
	}
	if (bIsFuel && bIsLighter) {
		LOG_EDITOR_WARNING("Item %s has both the fire fuel and light fire interfaces, light fire will override the behavior of the fire fuel interface, please remove it", *Item->GetItemName().ToString());
	}
	if (bIsLighter) {
		if (bIsBurning || TotalBurnTime <= 0.f) return false;
		float StartChance;
		ILightFireInterface::Execute_GetLightFireProperties(Item, StartChance);
		if (StartChance / 100.f <= FMath::FRand()) return false;
		bIsBurning = true;
		return true;
	}
	float BurnTime;
	IFireFuelInterface::Execute_GetFireFuelProperties(Item, BurnTime);
	if (BurnTime <= 0.f || TotalBurnTime >= MaxBurnTime) return false;
	TotalBurnTime = FMath::Clamp(TotalBurnTime + BurnTime, 0.f, MaxBurnTime);
	Player->PlayerInventory->RemoveItem(Player->PlayerInventory->GetSelectedSlot());
	return true;
}