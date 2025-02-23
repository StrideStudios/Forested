#include "Player/PlayerInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Player/FPlayer.h"
#include "Interfaces/SelectableInterface.h"
#include "Widget/PlayerWidget.h"
#include "Water/WaterActor.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PhysicsVolume.h"
#include "Player/PlayerInventory.h"
#include "Kismet/KismetSystemLibrary.h"

UPlayerInputComponent::UPlayerInputComponent() {
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

//TODO: right click dont work properly ;(
void UPlayerInputComponent::BeginPlay() {
	Super::BeginPlay();

	Player = CastChecked<AFPlayer>(GetOwner());
	
	EnableJump();
	EnableSprint();

	Player->GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	
	const APlayerController* PlayerController = Player->GetPlayerController();
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if (Subsystem && !InputContext.IsNull()) {
		Subsystem->AddMappingContext(InputContext.LoadSynchronous(), 0);
	}
	
	BindAction(MoveAction, ETriggerEvent::Triggered, this, &UPlayerInputComponent::Move);
	BindAction(MoveAction, ETriggerEvent::Completed, this, &UPlayerInputComponent::MoveStop);
	BindAction(LookAction, ETriggerEvent::Triggered, this, &UPlayerInputComponent::Look);
	BindAction(JumpAction, ETriggerEvent::Triggered, this, &UPlayerInputComponent::Jump);
	BindAction(CrouchAction, ETriggerEvent::Started, this, &UPlayerInputComponent::Crouch);
	BindAction(SprintAction, ETriggerEvent::Triggered, this, &UPlayerInputComponent::StartSprint);
	BindAction(SprintAction, ETriggerEvent::Completed, this, &UPlayerInputComponent::StopSprint);
	
	BindAction(LMBAction, ETriggerEvent::Triggered, this, &UPlayerInputComponent::LeftInteract);
	BindAction(RMBAction, ETriggerEvent::Triggered, this, &UPlayerInputComponent::RightInteract);
	BindAction(RMBAction, ETriggerEvent::Completed, this, &UPlayerInputComponent::EndRightInteract);
	BindAction(ButtonAction, ETriggerEvent::Triggered, this, &UPlayerInputComponent::ButtonInteract);
	BindAction(ButtonAction, ETriggerEvent::Completed, this, &UPlayerInputComponent::EndButtonInteract);
	BindAction(SetItemAction, ETriggerEvent::Started, this, &UPlayerInputComponent::SetItem);
	
	BindAction(MenuAction, ETriggerEvent::Started, this, &UPlayerInputComponent::MenuPressed);
	
	LOG("Player Input Initialized");
}

void UPlayerInputComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Distance to reach
	
	if (!Player || !Player->GetCharacterMovement()) return;

	const AWaterActor* WaterActor = nullptr;
	double WaterZHeight = TNumericLimits<float>::Lowest();
	FVector WaterUpVector = FVector(0.0);
	
	const FCollisionQueryParams CollisionQueryParams(SCENE_QUERY_STAT(DefaultQueryParam), false);
	TArray<FHitResult> Hits; 
	if (GetWorld()->SweepMultiByObjectType(Hits, Player->GetActorLocation(), Player->GetActorLocation(), FQuat::Identity, WATER_OBJECT_CHANNEL, FCollisionShape::MakeCapsule(Player->GetCapsuleComponent()->GetScaledCapsuleRadius(), Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), CollisionQueryParams)) {
		for (const FHitResult& Result : Hits) {
			if (const AWaterActor* HitActor = Cast<AWaterActor>(Result.GetActor())) {
				const UWaterSplineComponent* WaterSpline = HitActor->SplineComponent;
				check(WaterSpline);

				const float InputKey = WaterSpline->FindInputKeyClosestToWorldLocation(Player->Camera->GetComponentLocation());
				const float HitActorWaterZHeight = WaterSpline->GetLocationAtSplineInputKey(InputKey, ESplineCoordinateSpace::World).Z;
				if (HitActorWaterZHeight > WaterZHeight) {
					WaterActor = HitActor;
					WaterZHeight = HitActorWaterZHeight;
					WaterUpVector = WaterSpline->GetUpVectorAtSplineInputKey(InputKey, ESplineCoordinateSpace::World);
				}
			}
		}
	}
	
	if (WaterActor) {
		if (!bInWater) {
			bInWater = true;
			if (Player->GetVelocity().Z < -100.f && !Player->GetCharacterMovement()->IsMovingOnGround())
				bIsGoingBelowWater = true;
		}
	} else if (bInWater) {
		bInWater = false;
		SetNotSwimming();
	}

	if (!IsInWater()) return;
	
	const double WaistPoint = Player->WaistPoint->GetComponentLocation().Z;
	const double BreathingPoint = Player->BreathingPoint->GetComponentLocation().Z;
	
	if (!bSwimming) {
		if (WaistPoint <= WaterZHeight)
			SetSwimming();
		return;
	}

	
	//push player in the direction the water is moving, sin pushes the player periodically
	const float InputKey = WaterActor->SplineComponent->FindInputKeyClosestToWorldLocation(Player->GetActorLocation());
	Player->AddActorWorldOffset(WaterActor->GetWaterDirectionAtInputKey(InputKey) * WaterActor->GetWaterSpeedAtInputKey(InputKey) * WaterVelocityMultiplier);
	
	//this means push the character up above the surface
	if (bSwimming && (bIsGoingAboveWater || bIsGoingBelowWater)) {
		const float MovementValue = bIsGoingAboveWater ? WaterEnterSpeed : -WaterEnterSpeed;
		Player->AddActorLocalOffset(FVector(0.0, 0.0, MovementValue) * DeltaTime * 60.0);
		if (bIsGoingAboveWater ? WaistPoint < WaterZHeight : BreathingPoint > WaterZHeight) return;
		bIsAboveWater = bIsGoingAboveWater;
		bIsGoingAboveWater = false;
		bIsGoingBelowWater = false;
	}
	
	const double DotProduct = FVector::DotProduct(Player->Camera->GetForwardVector(), WaterUpVector);
	if (bIsAboveWater) {
		//above water prevent from going under if requirements not met
		if (MovementVector.Y > 0.0 && DotProduct <= EnterWaterMin) {
			bIsGoingBelowWater = true;
			return;
		}
		//this keeps the waist point at the location of the spline
		Player->AddActorLocalOffset(FVector(0.0, 0.0, WaterZHeight - WaistPoint));
	} else {
		//below water prevent from going above if requirements not met
		if (MovementVector.Y > 0.0 && DotProduct >= ExitWaterMin) {
			bIsGoingAboveWater = true;
			return;
		}
		if (BreathingPoint < WaterZHeight) return;
		//this keeps the breathing point below the location of the spline
		Player->AddActorLocalOffset(FVector(0.0, 0.0, WaterZHeight - BreathingPoint));
	}

	//5.0 is a buffer to allow some wiggle room for being considered 'swimming'
	if (WaistPoint - 5.0 > WaterZHeight) {
		SetNotSwimming();
	}
}

void UPlayerInputComponent::SetSwimming() {
	Uncrouch();
	bSwimming = true;
	Player->GetCharacterMovement()->GetPhysicsVolume()->bWaterVolume = true;
	bIsAboveWater = true;
	Player->GetCharacterMovement()->SetMovementMode(MOVE_Swimming);
}
//TODO: after initial button press it stops working
void UPlayerInputComponent::SetNotSwimming() {
	Uncrouch();
	bSwimming = false;
	Player->GetCharacterMovement()->GetPhysicsVolume()->bWaterVolume = false;
	bIsGoingBelowWater = false;
	bIsGoingAboveWater = false;
}

void UPlayerInputComponent::Move(const FInputActionValue& Value) {
	if (!Player->GetController() || !Player->GetCharacterMovement()) return;
	
	MovementVector = Value.Get<FVector2D>();

	if (bSwimming && !bSwimCanMove) return;
	
	const FVector ForwardDirection(bSwimming ? Player->Camera->GetForwardVector() : Player->GetPlayerForwardVector());
	const FVector RightDirection(bSwimming ? Player->Camera->GetRightVector() : Player->GetPlayerRightVector());
	
	Player->AddMovementInput(ForwardDirection, MovementVector.Y);
	Player->AddMovementInput(RightDirection, MovementVector.X);
	
	const FVector Start = Player->Camera->GetComponentLocation();
	const FVector End = Player->Camera->GetComponentLocation() - FVector(0.f, 0.f, 500.f);

	FHitResult HitResult;
	if (!UKismetSystemLibrary::LineTraceSingle(this, Start, End, UEngineTypes::ConvertToTraceType(ECC_Visibility), true, {}, EDrawDebugTrace::None, HitResult, true, FLinearColor::Yellow, FLinearColor::Red, 0.1)) return;
	PhysicalMaterial = HitResult.PhysMaterial.Get();
}

void UPlayerInputComponent::MoveStop(const FInputActionValue& Value) {
	MovementVector = FVector2D(0.0);
}

void UPlayerInputComponent::Look(const FInputActionValue& Value) {
	if (!Player->GetController()) return;
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	Player->AddControllerYawInput(LookAxisVector.X);
	Player->AddControllerPitchInput(LookAxisVector.Y);
}

void UPlayerInputComponent::Jump(const FInputActionValue& Value) {
	if (!Player->GetCharacterMovement()) return;
	if (bSwimming) {
		if (AllowUpMovement)
			Player->AddMovementInput(FVector(0.0, 0.0, 1.0));
		return;
	}
	if (!bCanJump) return;
	if (bInDeepWater) {
		Player->GetCharacterMovement()->JumpZVelocity = DeepWaterJumpVelocity;
		Player->Jump();
		StopSprint(FInputActionValue());
		return;
	}
	Player->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
	Player->Jump();
	StopSprint(FInputActionValue());
}

void UPlayerInputComponent::Crouch(const FInputActionValue& Value) {
	if (Uncrouch()) return;
	Player->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	Player->Crouch();
}

bool UPlayerInputComponent::Uncrouch() {
	if (!Player->bIsCrouched) return false;
	Player->UnCrouch();
	return true;
}

void UPlayerInputComponent::StartSprint(const FInputActionValue& Value) {
	if (Player->GetVelocity().Size() < WalkSpeed / 10.f || MovementVector.Y <= 0.0) {
		StopSprint(Value);
		return;
	}
	if (!bCanSprint || Player->bIsCrouched || !Player->GetCharacterMovement() || !Player->GetCharacterMovement()->IsMovingOnGround()) return;
	bSprinting = true;
	Player->GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void UPlayerInputComponent::StopSprint(const FInputActionValue& Value) {
	if (Player->bIsCrouched || !Player->GetCharacterMovement()) return;
	bSprinting = false;
	Player->GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void UPlayerInputComponent::LeftInteract(const FInputActionValue& Value) {
	if (bSwimming || bInDeepWater || !Player->PlayerInventory) return;
	Player->PlayerInventory->LeftInteract();
}

void UPlayerInputComponent::RightInteract(const FInputActionValue& Value) {
	if (!Player->PlayerInventory) return;
	if (bHoldingRightInteract && (bSwimming || bInDeepWater)) {
		Player->PlayerInventory->EndRightInteract();
		bHoldingRightInteract = false;
		return;
	}
	Player->PlayerInventory->RightInteract();
	bHoldingRightInteract = true;
}

void UPlayerInputComponent::EndRightInteract(const FInputActionValue& Value) {
	if (bSwimming || bInDeepWater || !Player->PlayerInventory) return;
	Player->PlayerInventory->EndRightInteract();
	bHoldingRightInteract = false;
}

void UPlayerInputComponent::ButtonInteract(const FInputActionValue& Value) {
	ButtonHeldTime++;
}

void UPlayerInputComponent::EndButtonInteract(const FInputActionValue& Value) {
	if (!Player->PlayerInventory) return;

	const FVector Start = Player->Camera->GetComponentLocation();
	const FVector End = Player->Camera->GetForwardVector() * MaxSelectDistance + Start;

	FCollisionQueryParams Params(TEXT("LineTraceSingle"), SCENE_QUERY_STAT_ONLY(ForestedSelectTrace), true);
	Params.AddIgnoredActor(Player);
	FHitResult SelectResult;
	if (GetWorld()->LineTraceSingleByChannel(SelectResult, Start, End, SELECT_TRACE_CHANNEL, Params)) {
		AActor* ActorToSelect = SelectResult.GetActor();
		if (AActor* ParentActor = SelectResult.GetActor()->GetParentActor())
			ActorToSelect = ParentActor;
		if (ActorToSelect->GetClass()->ImplementsInterface(USelectableInterface::StaticClass())) {
			if (ISelectableInterface::Execute_CanSelect(ActorToSelect, Player, SelectResult, ButtonHeldTime)) {
				if (ISelectableInterface::Execute_Selected(ActorToSelect, Player, SelectResult, ButtonHeldTime))
					if (!bSwimming && !bInDeepWater) Player->PlayerInventory->ButtonInteract();
			}
		}
	}
	ButtonHeldTime = 0.f;
}

void UPlayerInputComponent::SetItem(const FInputActionValue& Value) {
	if (!Player->PlayerInventory) return;
	const int i = Value.Get<float>() - 1.f;
	if (i != Player->PlayerInventory->GetSelectedSlot()) {
		Player->PlayerInventory->SetSelectedSlot(i);
	}
}

void UPlayerInputComponent::MenuPressed(const FInputActionValue& Value) {
	if (!Player->IsGameStarted() || Player->HandleMenuPressed()) return;
	if (Player->IsInMenu()) {
		Player->ResumeGame();
		return;
	}
	Player->PauseGame();
}

UPlayerMovementComponent::UPlayerMovementComponent() {
	NavAgentProps.bCanSwim = true;
	NavAgentProps.bCanCrouch = true;
	NavAgentProps.bCanJump = true;
	NavAgentProps.bCanWalk = true;
}

bool UPlayerMovementComponent::IsInWater() const {
	const AFPlayer* Player = Cast<AFPlayer>(GetOwner());
	if (!Player || !Player->PlayerInputComponent) return false;
	return Player->PlayerInputComponent->IsSwimming();
}
