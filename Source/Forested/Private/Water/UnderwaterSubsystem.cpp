#include "Water/UnderwaterSubsystem.h"
#include "Player/FPlayer.h"
#include "Serialization/SerializationLibrary.h"
#include "Water/WaterActor.h"
#include "Camera/CameraComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

bool UUnderwaterSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const {
	//is in world
	return WorldType == EWorldType::Game || WorldType == EWorldType::Editor || WorldType == EWorldType::PIE;
}

void UUnderwaterSubsystem::Initialize(FSubsystemCollectionBase& Collection) {
	UWorld* World = GetWorld();
	check(World);

	const TSoftObjectPtr<UMaterialParameterCollection> SoftMaterialParameterCollection(FSoftObjectPath(TEXT("/Game/Geometry/ParameterCollections/MPC_Water.MPC_Water")));
	FSerializationLibrary::LoadSync(SoftMaterialParameterCollection);
	MaterialParameterCollection = SoftMaterialParameterCollection.Get();
	
	World->OnBeginPostProcessSettings.AddUObject(this, &UUnderwaterSubsystem::ComputeUnderwaterPostProcess);
	World->InsertPostProcessVolume(&UnderwaterPostProcessVolume);
}

void UUnderwaterSubsystem::Deinitialize() {
	UWorld* World = GetWorld();
	check(World);
	World->OnBeginPostProcessSettings.RemoveAll(this);
	World->RemovePostProcessVolume(&UnderwaterPostProcessVolume);
}

void UUnderwaterSubsystem::ComputeUnderwaterPostProcess(const FVector ViewLocation, FSceneView* SceneView) {
	UWorld* World = GetWorld();
	if (!World) return;
	constexpr float TraceDistance = 85.f;
	const FCollisionQueryParams CollisionQueryParams(SCENE_QUERY_STAT(DefaultQueryParam), false);

	WaterActor = nullptr;
	TArray<FHitResult> Hits; 
	if (World->SweepMultiByObjectType(Hits, ViewLocation, ViewLocation + FVector(0, 0, TraceDistance), FQuat::Identity, WATER_OBJECT_CHANNEL, FCollisionShape::MakeSphere(TraceDistance), CollisionQueryParams)) {
		float CachedWaterZHeight = TNumericLimits<float>::Lowest();
		for (const FHitResult& Result : Hits) {
			if (AWaterActor* HitActor = Cast<AWaterActor>(Result.GetActor())) {
				const UWaterSplineComponent* WaterSpline = HitActor->SplineComponent;
				check(WaterSpline);

				const float InputKey = WaterSpline->FindInputKeyClosestToWorldLocation(ViewLocation);
				const float WaterZHeight = WaterSpline->GetLocationAtSplineInputKey(InputKey, ESplineCoordinateSpace::World).Z;
				if (WaterZHeight > CachedWaterZHeight) {
					WaterActor = HitActor;
					CachedWaterZHeight = WaterZHeight;
				}
			}
		}
	}

	if (WaterActor) {
		const UWaterSplineComponent* WaterSpline = WaterActor->SplineComponent;
		check(WaterSpline);

		const FMatrix InvViewProjMatrix = SceneView->ViewMatrices.GetInvViewProjectionMatrix();

		//get the location of the near plane on the left and right of the screen
		const FVector RightLocation = SceneView->NearClippingDistance * InvViewProjMatrix.TransformPosition(FVector(1.f, 0.f, 1.f));
		const FVector LeftLocation = SceneView->NearClippingDistance * InvViewProjMatrix.TransformPosition(FVector(-1.f, 0.f, 1.f));

		const float InputKey = WaterSpline->FindInputKeyClosestToWorldLocation(ViewLocation);
		const float WaterZHeight = WaterSpline->GetLocationAtSplineInputKey(InputKey, ESplineCoordinateSpace::World).Z;
				
		LeftZHeight = WaterSpline->FindLocationClosestToWorldLocation(LeftLocation, ESplineCoordinateSpace::World).Z;
		RightZHeight = WaterSpline->FindLocationClosestToWorldLocation(RightLocation, ESplineCoordinateSpace::World).Z;
		CameraDepth = ViewLocation.Z - WaterZHeight;

		UnderwaterPostProcessVolume.PostProcessProperties = WaterSpline->GetPostProcessProperties();

		bIsInWater = true;
		UpdateSceneProperties(World, SceneView);
		return;
	}

	if (bIsInWater) {
		SetNotInWater();
		UpdateSceneProperties(World, SceneView);
	}
}



void UUnderwaterSubsystem::UpdateSceneProperties(UWorld* World, FSceneView* SceneView) {
	SceneView->UnderwaterDepth = CameraDepth;
	
	if (!MaterialParameterCollection) return;
	UMaterialParameterCollectionInstance* MaterialParameterCollectionInstance = World->GetParameterCollectionInstance(MaterialParameterCollection);
	MaterialParameterCollectionInstance->SetScalarParameterValue("CameraDepth", CameraDepth);
	MaterialParameterCollectionInstance->SetScalarParameterValue("LeftZHeight", LeftZHeight);
	MaterialParameterCollectionInstance->SetScalarParameterValue("RightZHeight", RightZHeight);
}

void UUnderwaterSubsystem::SetNotInWater() {
	bIsInWater = false;

	WaterActor = nullptr;
	CameraDepth = -1.f;
	LeftZHeight = 0.f;
	RightZHeight = 0.f;
	
	UnderwaterPostProcessVolume.PostProcessProperties.bIsEnabled = false;
	UnderwaterPostProcessVolume.PostProcessProperties.Settings = nullptr;
}

