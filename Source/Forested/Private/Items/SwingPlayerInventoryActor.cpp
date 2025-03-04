#include "Items/SwingPlayerInventoryActor.h"
#include "Interfaces/DamageableInterface.h"
#include "Player/FPlayer.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Animation/AnimInstance.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

ASwingPlayerInventoryActor::ASwingPlayerInventoryActor() {
	PrimaryActorTick.bCanEverTick = false;
	DamageType = EDamageType::Axe;
	
	StartTraceMovement = CreateDefaultSubobject<USceneComponent>(TEXT("Start Trace Movement"));
	StartTraceMovement->SetupAttachment(GetRootComponent());
	StartTrace = CreateDefaultSubobject<USceneComponent>(TEXT("Start Trace"));
	StartTrace->SetupAttachment(StartTraceMovement);
	EndTrace = CreateDefaultSubobject<USceneComponent>(TEXT("End Trace"));
	EndTrace->SetupAttachment(GetRootComponent());
	WeaponSwing = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Weapon Swing"));
	WeaponSwing->SetupAttachment(GetRootComponent());
	WeaponSwing->SetVisibility(false);
}

void ASwingPlayerInventoryActor::InventoryTick(const float DeltaTime) {
	if (Delay > 0.f) Delay = Delay - DeltaTime;
	WeaponSwing->SetVisibility(CanHit());
	if (CanHit()) {
		switch (bHit) {
		case true: break;
		case false:
			FHitResult HitResult;
			TArray<AActor*> ActorsToIgnore;
			ActorsToIgnore.Add(this);
			ActorsToIgnore.Add(PLAYER);

			if (!UKismetSystemLibrary::SphereTraceSingle(this, StartTrace->GetComponentLocation(), EndTrace->GetComponentLocation(), 4.f, UEngineTypes::ConvertToTraceType(HIT_TRACE_CHANNEL), false, ActorsToIgnore, EDrawDebugTrace::None, HitResult, true))
				break;
			if (!HitResult.GetComponent()->IsPhysicsCollisionEnabled())
				break;
			if (HitResult.GetComponent()->IsAnySimulatingPhysics())
				HitResult.GetComponent()->AddImpulseAtLocation(HitResult.ImpactNormal * -1.f, HitResult.Location);
			PauseMontage(SwingMontage);
            StopMontage(HitBlend, SwingMontage);
            WeaponSwing->SetVisibility(false);
			PLAYER->EnablePlayerMovement();
			//PLAYER->ResetMeshSize();
			bHit = true;
			bCanHit = false;
			Delay = HitDelay;
			WeaponSwing->SetVisibility(false);
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitNiagaraSystem, HitResult.Location, UKismetMathLibrary::MakeRotFromX(HitResult.ImpactNormal));
			UDamageLibrary::ApplyDamage(HitResult.GetActor(), PLAYER, HitResult, Damage, DamageType);
			break;
		}
	}
	StartTraceMovement->SetWorldTransform(PTraceTransform);
	PTraceTransform = RootComponent->GetComponentTransform();
	Super::InventoryTick(DeltaTime);
}

void ASwingPlayerInventoryActor::OnMontageNotifyBegin(const UAnimMontage* Montage, const FName Notify) {
	Super::OnMontageNotifyBegin(Montage, Notify);
	if (Notify == "CanHit") {
		bCanHit = true;
	}
}

void ASwingPlayerInventoryActor::OnMontageNotifyEnd(const UAnimMontage* Montage, const FName Notify) {
	Super::OnMontageNotifyEnd(Montage, Notify);
	if (Notify == "CanHit") {
		bCanHit = false;
	}
}

void ASwingPlayerInventoryActor::OnMontageBlendOut(const UAnimMontage* Montage, const bool bInterrupted) {
	if (SwingMontage && SwingMontage == Montage) {
		WeaponSwing->SetVisibility(false);
		PLAYER->EnablePlayerMovement();
	}
}

bool ASwingPlayerInventoryActor::CanMontagePlay_Implementation(const UAnimMontage* Montage, const float PlayRate, const float StartingPosition) const {
	return !IsSwinging() && CanSwing();
}

bool ASwingPlayerInventoryActor::Swing(UAnimMontage* Montage, const FAlphaBlend& InHitBlend, const float InHitDelay, const float PlayRate, const float StartingPosition, const bool bStacked) {
	if (bStacked ? StartStackedMontage(Montage, PlayRate, StartingPosition) : StartMontage(Montage, PlayRate, StartingPosition)) {
		SwingMontage = Montage;
		HitBlend = InHitBlend;
		HitDelay = InHitDelay;
		bHit = false;
		PTraceTransform = RootComponent->GetComponentTransform();
		PLAYER->DisablePlayerMovement();
		return true;
	}
	return false;
}

bool ASwingPlayerInventoryActor::IsSwinging() const {
	if (!SwingMontage) return false;
	if (const UAnimInstance* AnimInstance = PLAYER->GetMesh()->GetAnimInstance()) {
		return AnimInstance->Montage_IsActive(SwingMontage);
	}
	return false;
}
