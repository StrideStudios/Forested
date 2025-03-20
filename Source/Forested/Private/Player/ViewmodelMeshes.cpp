#include "Player/ViewmodelMeshes.h"
#include "Camera/CameraComponent.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Player/FPlayer.h"

//https://sahildhanju.com/posts/render-first-person-fov/
//https://github.com/tiger-punch-sports-club/ue4-fps-double-camera-separate-fov/blob/master/Source/FovTutorial/MySkeletalMeshComponent.cpp
//https://github.com/tiger-punch-sports-club/ue4-fps-double-camera-separate-fov/blob/master/Source/FovTutorial/DarkMagic/Utils.h
//Sahil Dhanju

FViewmodelVector::operator FVector() const {
	return ConvertToVector(ViewmodelData, Vector);
}

FVector FViewmodelVector::ConvertToVector(const FViewmodelData ViewmodelData, const FVector Vector) {
	FMatrix Matrix = FTransform(Vector).ToMatrixWithScale();
	UViewmodelMeshes::CalculateViewmodelMatrix(PLAYER->GetPlayerController(), ViewmodelData, Matrix);
	return FTransform(Matrix).GetLocation();
}

FViewmodelRotator::operator FQuat() const {
	return ConvertToQuat(ViewmodelData, Rotation.Quaternion());
}

FQuat FViewmodelRotator::ConvertToQuat(const FViewmodelData ViewmodelData, const FQuat Rotation) {
	FMatrix Matrix = FTransform(Rotation).ToMatrixWithScale();
	UViewmodelMeshes::CalculateViewmodelMatrix(PLAYER->GetPlayerController(), ViewmodelData, Matrix);
	return FTransform(Matrix).GetRotation();
}

FViewmodelTransform::operator FTransform() const {
	return ConvertToTransform(ViewmodelData, Transform);
}

FTransform FViewmodelTransform::ConvertToTransform(const FViewmodelData ViewmodelData, const FTransform Transform) {
	FMatrix Matrix = Transform.ToMatrixWithScale();
	UViewmodelMeshes::CalculateViewmodelMatrix(PLAYER->GetPlayerController(), ViewmodelData, Matrix);
	return FTransform(Matrix);
}

FVector UViewmodelMeshes::Conv_ViewmodelVectorToVector(const FViewmodelVector& Vector) {
	return FVector(Vector);
}

FRotator UViewmodelMeshes::Conv_ViewmodelRotatorToRotator(const FViewmodelRotator& Rotator) {
	return FQuat(Rotator).Rotator();
}

FQuat UViewmodelMeshes::Conv_ViewmodelRotatorToQuat(const FViewmodelRotator& Rotator) {
	return FQuat(Rotator);
}

FTransform UViewmodelMeshes::Conv_ViewmodelTransformToTransform(const FViewmodelTransform& Transform) {
	return FTransform(Transform);
}

void UViewmodelMeshes::CalculateViewmodelMatrix(const APlayerController* PlayerController, const FViewmodelData& ViewmodelData, FMatrix& InOutMatrix) {
	if (!PlayerController) return;
	const ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PlayerController->Player);
	if (!LocalPlayer || !LocalPlayer->ViewportClient || !LocalPlayer->ViewportClient->Viewport) return;

	FSceneViewProjectionData ProjectionData;
	if (!LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport, eSSP_FULL, ProjectionData)) return;

	const FMatrix ViewMatrix = ProjectionData.ComputeViewProjectionMatrix();

	FMatrix PerspectiveAdjustmentMatrix = ProjectionData.ProjectionMatrix;

	constexpr double TargetAspect = 1920.f/1080.f;
	
	const double CurrentAspect = ProjectionData.ProjectionMatrix.M[1][1] / ProjectionData.ProjectionMatrix.M[0][0];
	
	if (ViewmodelData.bUseViewmodelFOV) {

		const float NearClippingPlaneDistance = ProjectionData.ProjectionMatrix.M[3][2];
		
		const double CorrectedFOV = CorrectFOVRad(ViewmodelData.ViewmodelFOV, TargetAspect, CurrentAspect);
		
		PerspectiveAdjustmentMatrix = FMatrix(
				FPlane(1.0 / CorrectedFOV, 0.0, 0.0, 0.0),
				FPlane(0.0, CurrentAspect / CorrectedFOV, 0.0, 0.0),
				FPlane(0.0, 0.0, 0.0, 1.0),
				FPlane(0.0, 0.0, NearClippingPlaneDistance, 0.0));
		
	}

	FMatrix ViewPerspectiveAdjustmentMatrix = ViewMatrix * PerspectiveAdjustmentMatrix;
	
	if (ViewmodelData.bUseViewmodelScale) {
		const FMatrix ScaleAdjustmentMatrix = FMatrix(
							FPlane(ViewmodelData.ViewmodelScale, 0.0, 0.0, 0.0),
							FPlane(0.0, ViewmodelData.ViewmodelScale, 0.0, 0.0),
							FPlane(0.0, 0.0, ViewmodelData.ViewmodelScale, 0.0),
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
	if (!ViewmodelData.bUseViewmodelFOV && !ViewmodelData.bUseViewmodelScale) return Matrix;
	UViewmodelMeshes::CalculateViewmodelMatrix(PlayerController, ViewmodelData, Matrix);
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
	if (!ViewmodelData.bUseViewmodelFOV && !ViewmodelData.bUseViewmodelScale) return Matrix;
	UViewmodelMeshes::CalculateViewmodelMatrix(PlayerController, ViewmodelData, Matrix);
	return Matrix;
}