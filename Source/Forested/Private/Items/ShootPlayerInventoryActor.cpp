#include "Items/ShootPlayerInventoryActor.h"

#include "NiagaraFunctionLibrary.h"
#include "Interfaces/DamageableInterface.h"
#include "Player/FPlayer.h"

AShootPlayerInventoryActor::AShootPlayerInventoryActor() {
	PrimaryActorTick.bCanEverTick = false;
	
}

bool AShootPlayerInventoryActor::Shoot(UAnimMontage* Montage, const FViewmodelVector ShootLocation, const float PlayRate, const float StartingPosition, const bool Aimed) {
	if (StartMontage(Montage, PlayRate, StartingPosition)) {
		
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
//TODO: way to inject stuff into player hud
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
