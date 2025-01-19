#include "WaterActor.h"

#include "FPlayer.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h" 
#include "Forested/Forested.h"

UWaterSplineComponent::UWaterSplineComponent() {
	PrimaryComponentTick.bCanEverTick = false;
	
	CollisionObjectType = WATER_OBJECT_CHANNEL;
	CollisionType = ECollisionEnabled::QueryOnly;
	OverlapEvents = true;
}

void UWaterSplineComponent::BeginPlay() {
	Super::BeginPlay();
}

//TODO: Rotation of actor affects tangents of spline

//TODO: water has tiny gaps in-between components

//TODO: water movement like stuff in river

void UWaterSplineComponent::RefreshSplineComponent() {
	const int Sections = GetNumberOfSplineComponents();
	//create waterTypes for the sections with no current water type or delete if too many are created
	while (WaterData.Num() != Sections) {
		if (WaterData.Num() > Sections) {
			WaterData.RemoveAt(WaterData.Num() - 1);
			continue;
		}
		if (WaterData.Num() < Sections) {
			WaterData.Add(FWaterData());
		}
	}
	bShouldVisualizeScale = true;
	ScaleVisualizationWidth = GetComponentScale().Y * 100.0;
	Super::RefreshSplineComponent();
}

void UWaterSplineComponent::InitComponent(USplineMeshComponent* SplinePointMesh, const int Index) {
	Super::InitComponent(SplinePointMesh, Index);
	SplinePointMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	//set materials from the material map
	if (!MaterialMap.Num() || !WaterData.IsValidIndex(Index) || !MaterialMap.Contains(WaterData[Index].WaterType)) return;
	TArray<UMaterialInterface*> MaterialArray = MaterialMap[WaterData[Index].WaterType].Materials;
	for (int v = 0; v < MaterialArray.Num(); v++) {
		SplinePointMesh->SetMaterial(v, MaterialArray[v]);
	}
}

AWaterActor::AWaterActor() {
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	SplineComponent = CreateDefaultSubobject<UWaterSplineComponent>(TEXT("River Spline Component"));
	RootComponent = SplineComponent;
}

void AWaterActor::BeginPlay() {
	Super::BeginPlay();
	SplineComponent->RefreshSplineComponent();
}

void AWaterActor::Tick(const float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
}

void AWaterActor::OnConstruction(const FTransform& Transform) {
	Super::OnConstruction(Transform);
	SetActorRotation(FRotator(0.0));
}

float AWaterActor::GetRiverAmountAtInputKey(const float InputKey) const {
	if (!SplineComponent->WaterData.IsValidIndex(InputKey)) return 0.f;
	
	const float PeriodAlongInputKey = FMath::Frac(InputKey);
	const FWaterData Data = SplineComponent->WaterData[InputKey];
	switch (Data.WaterType) {
	case EWaterType::Lake:
		return 0.f;
	case EWaterType::River:
		return 1.f;
	case EWaterType::LakeToRiver:
		return PeriodAlongInputKey;
	case EWaterType::RiverToLake:
		return 1.f - PeriodAlongInputKey;
	}
	return -1.f;
}

FVector AWaterActor::GetWaterDirectionAtInputKey(const float InputKey) const {
	if (!SplineComponent->WaterData.IsValidIndex(InputKey)) return FVector::ZeroVector;

	const float PeriodAlongInputKey = FMath::Frac(InputKey);
	const FVector SplineDirection = SplineComponent->GetDirectionAtSplineInputKey(InputKey, ESplineCoordinateSpace::World);
	const FVector WindDirection = FVector(1.f, 0.5f, 0.f);//somehow get wind direction

	const FWaterData Data = SplineComponent->WaterData[InputKey];
	switch (Data.WaterType) {
	case EWaterType::Lake:
		return WindDirection;
	case EWaterType::River:
		return SplineDirection;
	case EWaterType::LakeToRiver:
		return FMath::Lerp(WindDirection, SplineDirection, PeriodAlongInputKey);
	case EWaterType::RiverToLake:
		return FMath::Lerp(SplineDirection, WindDirection, PeriodAlongInputKey);
	}
	return FVector::ZeroVector;
}

float AWaterActor::GetWaterSpeedAtInputKey(const float InputKey) const {
	if (!SplineComponent->WaterData.IsValidIndex(InputKey)) return 0.f;
	
	const FWaterData Data = SplineComponent->WaterData[InputKey];
	switch (Data.WaterType) {
	case EWaterType::Lake: case EWaterType::River:
		return Data.WaterSpeed;
	default:
		break;
	}
	if (!SplineComponent->WaterData.IsValidIndex(InputKey + 1.f)) {
		return Data.WaterSpeed;
	}
	
	const float PeriodAlongInputKey = FMath::Frac(InputKey);
	return FMath::Lerp(Data.WaterSpeed, SplineComponent->WaterData[InputKey + 1.f].WaterSpeed, PeriodAlongInputKey);
}