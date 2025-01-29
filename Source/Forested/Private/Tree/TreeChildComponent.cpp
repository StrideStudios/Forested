#include "Tree/TreeChildComponent.h"
#include "CullStaticMeshComponent.h"
#include "LevelDefaults.h"
#include "NiagaraFunctionLibrary.h"
#include "SerializationLibrary.h"
#include "Item/ItemActor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Tree/TreeChildActor.h"

void UTreeChildComponent::BeginPlay() {
	Super::BeginPlay();
}

void UTrunkComponent::Init(UTreeItem* InItem, const TSoftObjectPtr<UStaticMesh>& InStaticMesh) {
	Item = InItem;
	FSerializationLibrary::LoadAsync(InStaticMesh, [this, InStaticMesh] {
		if (!InStaticMesh.IsValid()) return;
		SetStaticMesh(InStaticMesh.Get());
	});
}

bool UTrunkComponent::SpawnItemActor() {
	//tree index will be set by Tree Split Actor with Init as declared above
	AItemActor::SpawnItemActor(GetWorld(), GetComponentTransform(), Item);
	return true;
}

bool UBranchComponent::SpawnItemActor() {
	//TODO: tree index?
	AItemActor::SpawnItemActor(GetWorld(), GetComponentTransform(), Item);
	return true;
}

bool UProductComponent::SpawnItemActor() {
	if (GetCurrentGrowth() <= 0.f) return false;
	if (GetCurrentGrowth() <= RequiredGrowth) {
		if (FallNiagaraSystem)
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FallNiagaraSystem, GetComponentLocation(), GetComponentRotation(), GetComponentScale());
		return true;
	}
	Item->CurrentGrowth = GetCurrentGrowth();
	AItemActor::SpawnItemActor(GetWorld(), GetComponentTransform(), Item);
	return true;
}

float UProductComponent::GetCurrentSize() const {
	if (StartGrowthTime == 0.f && EndGrowthTime == 0.f) return 1.f;
	return UKismetMathLibrary::MapRangeClamped(SKY->GetDaytimePassed(), StartGrowthTime, EndGrowthTime, 0.f, 1.f);
}

float UProductComponent::GetCurrentGrowth() const {
	if (StartGrowthTime == 0.f && EndGrowthTime == 0.f) return 100.f;
	return UKismetMathLibrary::MapRangeClamped(SKY->GetDaytimePassed(), StartGrowthTime, EndGrowthTime, 0.f, 100.f);
}

void ATreeItemActor::Init() {
	Super::Init();
	if (const UTreeProductItem* ProductItem = Cast<UTreeProductItem>(GetItem())) {
		StaticMeshComponent->SetCustomPrimitiveDataFloat(0, ProductItem->CurrentGrowth / 100.f);
	}
	StaticMeshComponent->SetCustomPrimitiveDataFloat(1, 1.f);
}
