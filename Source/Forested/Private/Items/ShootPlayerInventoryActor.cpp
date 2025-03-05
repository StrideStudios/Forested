#include "Items/ShootPlayerInventoryActor.h"
#include "NiagaraFunctionLibrary.h"
#include "Interfaces/DamageableInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Player/FPlayer.h"
#include "Player/PlayerHud.h"
#include "Player/PlayerInventory.h"
#include "Serialization/SerializationLibrary.h"

AShootPlayerInventoryActor::AShootPlayerInventoryActor() {
	PrimaryActorTick.bCanEverTick = true;
	SetTickGroup(TG_PostUpdateWork);
}

void AShootPlayerInventoryActor::Init() {
	Super::Init();
	if (WidgetClass.IsNull()) return;
	FSerializationLibrary::LoadSync(WidgetClass);
	ShootWidget = CreateWidget<UShootWidget>(PLAYER->GetPlayerController(), WidgetClass.Get());
	ShootWidget->AddToViewport(-1);
}

void AShootPlayerInventoryActor::Deinit() {
	Super::Deinit();
	if (!ShootWidget) return;
	ShootWidget->RemoveFromParent();
}

void AShootPlayerInventoryActor::OnMontageBlendOut(const UAnimMontage* Montage, const bool bInterrupted) {
	Super::OnMontageBlendOut(Montage, bInterrupted);
	if (ShootMontage && ShootMontage == Montage) {
		PLAYER->EnablePlayerMovement();
		EndShot();
	}
}

bool AShootPlayerInventoryActor::TraceShot(FHitResult& OutHit, const FVector ShootLocation, const float Rotation, const float InSpread) {
	if (!GetWorld() || !GetMesh()) return false;

	const float TrueSpread = InSpread * Range;
	const float TraceDistanceX = FMath::Cos(Rotation) * TrueSpread;
	const float TraceDistanceY = FMath::Sin(Rotation) * TrueSpread;
	const FVector TraceLocationX = GetMesh()->GetRightVector() * TraceDistanceX;
	const FVector TraceLocationY = GetMesh()->GetUpVector() * TraceDistanceY;

	const FVector EndDistance = GetMesh()->GetForwardVector() * Range;

	const FVector EndLocation = ShootLocation + EndDistance + TraceLocationX + TraceLocationY;

	FCollisionQueryParams Params;
	Params.bFindInitialOverlaps = false;
	Params.bReturnFaceIndex = false;
	Params.bReturnPhysicalMaterial = false;
	Params.bTraceComplex = true;
	Params.bIgnoreTouches = true;
	
	return GetWorld()->LineTraceSingleByChannel(OutHit, ShootLocation, EndLocation, ECC_Visibility, Params);
}

bool AShootPlayerInventoryActor::Shoot(UAnimMontage* Montage, const FViewmodelVector ShootLocation, const float PlayRate, const float StartingPosition, const bool Aimed) {
	if (StartMontage(Montage, PlayRate, StartingPosition)) {
		ShootMontage = Montage;
		
		PLAYER->AddControllerPitchInput(-0.1f);
		
		PLAYER->DisablePlayerMovement();
		
		for (int i = 0; i < AmountOfProjectiles; i++) {
			
			const float RandomRotation = FMath::RandRange(0.0, 2 * PI);
			const float RandomSpread = FMath::RandRange(0.0, Aimed ? Spread.Y : Spread.X);

			FHitResult HitResult;
			if (TraceShot(HitResult, ShootLocation, RandomRotation, RandomSpread)) {
				float Damage = Aimed ? MaxDamage.Y : MaxDamage.X;
				Damage = (1.f - HitResult.Distance / Range) * Damage;
				UDamageLibrary::ApplyDamage(HitResult.GetActor(), PLAYER, HitResult, Damage, EDamageType::Gun);
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitNiagaraSystem, HitResult.Location, UKismetMathLibrary::MakeRotFromX(HitResult.ImpactNormal));
			}
		}
		
		return true;
	}
	return false;
}

bool AShootPlayerInventoryActor::Aim(UAnimMontage* Montage, const float PlayRate, const float StartingPosition) {
	if (StartMontage(Montage, PlayRate, StartingPosition)) {
		if (ShootWidget) {
			ShootWidget->OnAim(this);
		}
		GetPlayerAnimInstance()->LeanIntensity = LeanIntensity;
		GetPlayerAnimInstance()->TargetPoseBlend = 1.f;
		return true;
	}
	return false;
}

bool AShootPlayerInventoryActor::UnAim(UAnimMontage* Montage) {
	if (ShootWidget) {
		ShootWidget->OnUnaim(this);
	}
	GetPlayerAnimInstance()->LeanIntensity = FVector2D(1.f);
	GetPlayerAnimInstance()->TargetPoseBlend = 0.f;
	if (ResumeMontage(Montage)) {
		return true;
	}
	StopMontage(0.25f, Montage);
	return false;
}

void AShootPlayerInventoryActor::TransformWidgetToShootPoint(const float DeltaSeconds, const FVector ShootLocation) {
	FViewmodelVector ViewmodelShootPoint;
	ViewmodelShootPoint.Vector = ShootLocation;
	ViewmodelShootPoint.ViewmodelData = GetViewmodelData();

	FHitResult HitResult;
	const bool bHit = TraceShot(HitResult, ViewmodelShootPoint, 0.f, 0.f);
	const FVector TargetLocation = bHit ? HitResult.Location : ShootLocation + GetMesh()->GetForwardVector() * Range;

	FVector2D TargetTranslation;
	if (UGameplayStatics::ProjectWorldToScreen(PLAYER->GetPlayerController(), TargetLocation, TargetTranslation, true)) {
		if (ShootWidgetTranslation.IsZero()) {
			ShootWidgetTranslation = TargetTranslation;
		}

		ShootWidgetTranslation = FMath::Vector2DInterpTo(ShootWidgetTranslation, TargetTranslation, DeltaSeconds, 35.f);

		if (ShootWidget)
			ShootWidget->TransformWidgetToShootPoint(ShootWidgetTranslation);
	}
}
