#include "ViewmodelMeshes.h"

#include "FPlayer.h"
#include "Camera/CameraComponent.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"

//https://sahildhanju.com/posts/render-first-person-fov/
//https://github.com/tiger-punch-sports-club/ue4-fps-double-camera-separate-fov/blob/master/Source/FovTutorial/MySkeletalMeshComponent.cpp
//https://github.com/tiger-punch-sports-club/ue4-fps-double-camera-separate-fov/blob/master/Source/FovTutorial/DarkMagic/Utils.h
//Sahil Dhanju

void FViewmodelMeshes::CalculateViewmodelMatrix(const APlayerController* PlayerController, const bool bUseViewmodelFOV, const float ViewmodelFOV, const bool bUseViewmodelScale, const float ViewmodelScale, FMatrix& InOutMatrix) {
	if (!PlayerController) return;
	const ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PlayerController->Player);
	if (!LocalPlayer || !LocalPlayer->ViewportClient || !LocalPlayer->ViewportClient->Viewport) return;

	FSceneViewProjectionData ProjectionData;
	if (!LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport, eSSP_FULL, ProjectionData)) return;

	const FMatrix ViewMatrix = ProjectionData.ComputeViewProjectionMatrix();

	FMatrix PerspectiveAdjustmentMatrix = ProjectionData.ProjectionMatrix;

	constexpr double TargetAspect = 1920.f/1080.f;
	
	const double CurrentAspect = ProjectionData.ProjectionMatrix.M[1][1] / ProjectionData.ProjectionMatrix.M[0][0];
	
	if (bUseViewmodelFOV) {

		const float NearClippingPlaneDistance = ProjectionData.ProjectionMatrix.M[3][2];
		
		const double CorrectedFOV = CorrectFOVRad(ViewmodelFOV, TargetAspect, CurrentAspect);
		
		PerspectiveAdjustmentMatrix = FMatrix(
				FPlane(1.0 / CorrectedFOV, 0.0, 0.0, 0.0),
				FPlane(0.0, CurrentAspect / CorrectedFOV, 0.0, 0.0),
				FPlane(0.0, 0.0, 0.0, 1.0),
				FPlane(0.0, 0.0, NearClippingPlaneDistance, 0.0));
		
	}

	FMatrix ViewPerspectiveAdjustmentMatrix = ViewMatrix * PerspectiveAdjustmentMatrix;
	
	if (bUseViewmodelScale) {
		const FMatrix ScaleAdjustmentMatrix = FMatrix(
							FPlane(ViewmodelScale, 0.0, 0.0, 0.0),
							FPlane(0.0, ViewmodelScale, 0.0, 0.0),
							FPlane(0.0, 0.0, ViewmodelScale, 0.0),
							FPlane(0.0, 0.0, 0.0, 1.0));

		ViewPerspectiveAdjustmentMatrix *= ScaleAdjustmentMatrix;
	}
	
	const FMatrix InverseViewProjectionMatrix = ProjectionData.ProjectionMatrix.Inverse() * ViewMatrix.Inverse();
	InOutMatrix *= ViewPerspectiveAdjustmentMatrix * InverseViewProjectionMatrix;
}

UViewmodelStaticMeshComponent::UViewmodelStaticMeshComponent() {
	SetBoundsScale(5.f);
}

void UViewmodelStaticMeshComponent::BeginPlay() {
	Super::BeginPlay();

	//TODO: change this stuff lol
	PlayerController = UGameplayStatics::GetPlayerController(this, 0);
}

FMatrix UViewmodelStaticMeshComponent::GetRenderMatrix() const {
	FMatrix Matrix = Super::GetRenderMatrix();
	if (!bUseViewmodelFOV && !bUseViewmodelScale) return Matrix;
	FViewmodelMeshes::CalculateViewmodelMatrix(PlayerController, bUseViewmodelFOV, ViewmodelFOV, bUseViewmodelScale, ViewmodelScale, Matrix);
	return Matrix;
}

UViewmodelSkeletalMeshComponent::UViewmodelSkeletalMeshComponent() {
	SetBoundsScale(5.f);
}

void UViewmodelSkeletalMeshComponent::BeginPlay() {
	Super::BeginPlay();

	//TODO: change this stuff lol
	PlayerController = UGameplayStatics::GetPlayerController(this, 0);
}

FMatrix UViewmodelSkeletalMeshComponent::GetRenderMatrix() const {
	FMatrix Matrix = Super::GetRenderMatrix();
	if (!bUseViewmodelFOV && !bUseViewmodelScale) return Matrix;
	FViewmodelMeshes::CalculateViewmodelMatrix(PlayerController, bUseViewmodelFOV, ViewmodelFOV, bUseViewmodelScale, ViewmodelScale, Matrix);
	return Matrix;
}