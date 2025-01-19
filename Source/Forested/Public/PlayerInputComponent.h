#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "PlayerInputComponent.generated.h"

class AWaterActor;
class AFPlayer;
class UInputMappingContext;

UCLASS(meta = (PrioritizeCategories = "Input Movement"))
class FORESTED_API UPlayerInputComponent : public UEnhancedInputComponent {
	GENERATED_BODY()

public:

	UPlayerInputComponent();

	//component is created right before setup, so setup can be done here instead
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetSwimming();

	void SetNotSwimming();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player Input Component")
	UPhysicalMaterial* GetPhysicalMaterial() const { return PhysicalMaterial; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player Input Component")
	EPhysicalSurface GetSurfaceType() const { return UPhysicalMaterial::DetermineSurfaceType(PhysicalMaterial); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player Input Component")
	void GetMovementVector(float& Right, float& Forward) const { Right = MovementVector.X; Forward = MovementVector.Y; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player Input Component")
	FORCEINLINE bool CanSprint() const { return bCanSprint; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player Input Component")
	FORCEINLINE bool IsSprinting() const { return bSprinting; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player Input Component")
	FORCEINLINE bool CanJump() const { return bCanJump; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player Input Component")
	FORCEINLINE bool IsSwimming() const { return bSwimming; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player Input Component")
	FORCEINLINE bool IsHoldingRightInteract() const { return bHoldingRightInteract; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player Input Component")
	FORCEINLINE bool IsInWater() const { return bInWater; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player Input Component")
	FORCEINLINE bool IsInDeepWater() const { return bInDeepWater; }

	UFUNCTION(BlueprintCallable, Category = "Player Input Component")
	void EnableSprint() {
		bCanSprint = true;
	}

	UFUNCTION(BlueprintCallable, Category = "Player Input Component")
	void DisableSprint() {
		bCanSprint = false;
		StopSprint({});
	}

	UFUNCTION(BlueprintCallable, Category = "Player Input Component")
	void EnableJump() {
		bCanJump = true;
	}

	UFUNCTION(BlueprintCallable, Category = "Player Input Component")
	void DisableJump() {
		bCanJump = false;
	}
	
	bool bSwimCanMove = false;
	
protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TSoftObjectPtr<UInputMappingContext> InputContext = nullptr;
	friend class AFPlayer;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	UInputAction* MoveAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	UInputAction* LookAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	UInputAction* CrouchAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	UInputAction* SprintAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	UInputAction* ButtonAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	UInputAction* LMBAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	UInputAction* RMBAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	UInputAction* SetItemAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	UInputAction* MenuAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	UInputAction* JumpAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float MaxSelectDistance = 10000.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float CrouchSpeed = 200.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float SprintSpeed = 600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float JumpVelocity = 700.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Water")
	bool AllowUpMovement = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Water")
	float WaterVelocityMultiplier = 0.01;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Water")
	float DeepWaterJumpVelocity = 400.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Water")
	float WaterTraceDistance = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Water")
	float WaterEnterSpeed = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Water", meta = (ClampMin = 0.0, ClampMax = 1.0, UIMin = 0.0, UIMax = 1.0))
	float EnterWaterMin = -0.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Water", meta = (ClampMin = 0.0, ClampMax = 1.0, UIMin = 0.0, UIMax = 1.0))
	float ExitWaterMin = 0.15f;
	
private:

	void Move(const FInputActionValue& Value);
	
	void MoveStop(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void Jump(const FInputActionValue& Value);

	void Crouch(const FInputActionValue& Value);

	bool Uncrouch();

	void StartSprint(const FInputActionValue& Value);

	void StopSprint(const FInputActionValue& Value);

	void LeftInteract(const FInputActionValue& Value);

	void RightInteract(const FInputActionValue& Value);

	void EndRightInteract(const FInputActionValue& Value);

	void ButtonInteract(const FInputActionValue& Value);

	void EndButtonInteract(const FInputActionValue& Value);

	void SetItem(const FInputActionValue& Value);

	void MenuPressed(const FInputActionValue& Value);

	UPROPERTY()
	AFPlayer* Player = nullptr;

	UPROPERTY()
	UPhysicalMaterial* PhysicalMaterial = nullptr;
	
	FVector2D MovementVector = FVector2D(0.0);

	FVector ClosestPoint = FVector(0.0);
	
	float ButtonHeldTime = 0.f;
	
	bool bCanSprint = true;
	
	bool bSprinting = false;

	bool bCanJump = true;

	bool bShouldSwim = false;
	
	bool bSwimming = false;

	bool bHoldingRightInteract = false;

	bool bIsAboveWater = true;
	
	bool bIsGoingAboveWater = false, bIsGoingBelowWater = false;

	bool bInWater = false, bInDeepWater = false;
	
};

UCLASS()
class FORESTED_API UPlayerMovementComponent : public UCharacterMovementComponent {
	GENERATED_BODY()

public:

	UPlayerMovementComponent();

	virtual bool IsInWater() const override;
	
};