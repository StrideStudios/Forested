#pragma once

#include "Forested/ForestedMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "FPlayer.generated.h"

class UWidget;
class UPlayerHud;
class UPlayerAnimInstance;
class UPawnWaterComponent;
class UPlayerInputComponent;
class USphereComponent;
class UCameraComponent;

DECLARE_DELEGATE_OneParam(FInterpolatePlayerDelegate, class AFPlayer*);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInterpolatePlayerDynamicDelegate, AFPlayer*, Player);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthUpdate, float, Health, float, Damage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMoneyUpdate, int, Money, int, Change);

UCLASS(Blueprintable, meta=(PrioritizeCategories = "Health Money Components"))
class FORESTED_API AFPlayer : public ACharacter {
	GENERATED_BODY()

	friend class UPlayerInventory;
	
public:
	
	AFPlayer(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;

public:
	
	virtual void Tick(float DeltaTime) override;

	virtual void OnStartCrouch(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust) override {
		Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
		GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, CrouchedEyeHeight));
	}

	virtual void OnEndCrouch(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust) override {
		Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
		GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, BaseEyeHeight));
	}
	
	virtual UInputComponent* CreatePlayerInputComponent() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Player")
	bool HandleMenuPressed();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player")
	FORCEINLINE APlayerController* GetPlayerController() const { return CastChecked<APlayerController>(GetController()); }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player")
	FORCEINLINE bool IsGameStarted() const { return bIsGameStarted; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player")
	FORCEINLINE FHitResult GetHoveredHitResult() const { return HoverHitResult; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player")
	FORCEINLINE FVector GetPlayerForwardVector() const { const FRotator YawRotation(0, GetController()->GetControlRotation().Yaw, 0); return FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player")
	FORCEINLINE FVector GetPlayerRightVector() const { const FRotator YawRotation(0, GetController()->GetControlRotation().Yaw, 0); return FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player")
	FORCEINLINE float GetForwardVelocity() const { return FVector::DotProduct(GetVelocity(), GetActorForwardVector()); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player")
	FORCEINLINE float GetRightVelocity() const { return FVector::DotProduct(GetVelocity(), GetActorRightVector()); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player")
	FORCEINLINE bool IsActorLoaded(AActor* Actor) const { return LoadedActors.Contains(Actor); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player|Health")
	FORCEINLINE float GetHealth() const { return Health; }
	
	UFUNCTION(BlueprintCallable, Category = "Player|Health")
	void DamagePlayer(const float Damage) {
		Health = FMath::Clamp(Health - Damage, 0.f, 100.f);
		OnHealthUpdate.Broadcast(Health, Damage);
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player|Money")
	FORCEINLINE int GetMoney() const { return Money; }
	
	UFUNCTION(BlueprintCallable, Category = "Player|Money")
	void AddMoney(const int Change) {
		Money += Change;
		OnMoneyUpdate.Broadcast(Money, Change);
	}

	UFUNCTION(BlueprintCallable, Category = "Inventory Render Actor|Player")
	void EnablePlayerMovement() const;
	
	UFUNCTION(BlueprintCallable, Category = "Inventory Render Actor|Player")
	void DisablePlayerMovement() const;
	
	UFUNCTION(BlueprintCallable, Category = "Player")
	bool GetPlayerAnimInstance(UPlayerAnimInstance*& OutAnimInstance) const;

	UFUNCTION(BlueprintCallable, Category = "Player")
	bool IsInputAllowed() const;
	
	UFUNCTION(BlueprintCallable, Category = "Player")
	void SetGameFocus() const;

	UFUNCTION(BlueprintCallable, Category = "Player")
	void SetUIFocus(UWidget* InWidgetToFocus = nullptr) const;

	UFUNCTION(BlueprintCallable, Category = "Player")
	void SetGameAndUIFocus(UWidget* InWidgetToFocus = nullptr) const;

	UFUNCTION(BlueprintCallable, Category = "Player")
	void ResetMousePosition() const;

	UFUNCTION(BlueprintImplementableEvent)
	void PlayFootstep();

	//interpolate the player's position to a world-space transform
	template<class UserClass>
	void InterpolatePlayerTo(const FTransform& Transform, const float Time = 1.f, const TEnumAsByte<EEasingFunc::Type> EasingFunc = EEasingFunc::EaseInOut, UserClass* InObj = nullptr, FInterpolatePlayerDelegate::TUObjectMethodDelegate<UserClass> OnUpdate = {}, FInterpolatePlayerDelegate::TUObjectMethodDelegate<UserClass> OnComplete = {}) {
		InterpolatePlayerTo_Internal(Transform, Time, EasingFunc, FInterpolatePlayerDelegate::CreateUObject(InObj, OnUpdate), FInterpolatePlayerDelegate::CreateUObject(InObj, OnComplete));
	}

	//interpolate the player's position to a world-space transform
	void InterpolatePlayerTo(const FTransform& Transform, const float Time = 1.f, const TEnumAsByte<EEasingFunc::Type> EasingFunc = EEasingFunc::EaseInOut, TFunction<void(AFPlayer*)> OnUpdate = {}, TFunction<void(AFPlayer*)> OnComplete = {}) {
		InterpolatePlayerTo_Internal(Transform, Time, EasingFunc, FInterpolatePlayerDelegate::CreateLambda(OnUpdate), FInterpolatePlayerDelegate::CreateLambda(OnComplete));
	}

	/*
	 * Hud Functions
	 */

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player")
	virtual bool IsInMenu() const;

	UFUNCTION(BlueprintCallable, Category = "Player")
	virtual void StartGame();

	UFUNCTION(BlueprintCallable, Category = "Player")
	virtual void ResumeGame() const;

	UFUNCTION(BlueprintCallable, Category = "Player")
	virtual void PauseGame() const;

	UFUNCTION(BlueprintCallable, Category = "Player")
	virtual void QuitGame() const;

	UFUNCTION(BlueprintCallable, Category = "Player")
	void ShowHud() const;
	
	UFUNCTION(BlueprintCallable, Category = "Player")
	void HideHud() const;
	
	UPROPERTY(BlueprintAssignable, Category = "Player|Health")
	FOnHealthUpdate OnHealthUpdate;

	UPROPERTY(BlueprintAssignable, Category = "Player|Money")
	FOnMoneyUpdate OnMoneyUpdate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPlayerInputComponent* PlayerInputComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPlayerInventory* PlayerInventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* BreathingPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* WaistPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UChildActorComponent* ItemMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* LoadMesh;

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget")
	TSoftClassPtr<UPlayerHud> PlayerHudClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	float Health = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Money")
	int Money = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float MaxHoverDistance = 10000.f;

private:

	//interpolate the player's position to a world-space transform
	void InterpolatePlayerTo_Internal(const FTransform& Transform, float Time = 1.f, TEnumAsByte<EEasingFunc::Type> EasingFunc = EEasingFunc::EaseInOut, FInterpolatePlayerDelegate OnUpdate = {}, FInterpolatePlayerDelegate OnComplete = {});

	void AddOnDestroyedListener(AActor* Actor);
	
	UFUNCTION()
	void LoadMeshOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void LoadMeshEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnActorSpawned(AActor* Actor);
	
	UFUNCTION()
	void OnActorRemoved(AActor* Actor);

	UPROPERTY()
	UPlayerHud* PlayerHud;
	
	UPROPERTY()
	TArray<AActor*> LoadedActors;

	UPROPERTY()
	TArray<AActor*> LoadedTickingActors;

	FHitResult HoverHitResult;

	bool bIsGameStarted = false;
	
};

UCLASS()
class FORESTED_API UInterpolatePlayerAsyncAction : public UBlueprintAsyncActionBase {
	GENERATED_BODY()

protected:

	/*
	 * parameters
	 */

	AFPlayer* Player;

	TEnumAsByte<EEasingFunc::Type> EasingFunc;

	float Time;

	FTransform Transform;

	UPROPERTY(BlueprintAssignable)
	FInterpolatePlayerDynamicDelegate Update;

	UPROPERTY(BlueprintAssignable)
	FInterpolatePlayerDynamicDelegate Complete;

	/*
	 * functions
	 */

	//Move the player to a specified location
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly = "true"), Category = "Player|Interpolation")
	static UInterpolatePlayerAsyncAction* InterpolatePlayerTo(AFPlayer* Player, FTransform Transform, float Time = 1.f, TEnumAsByte<EEasingFunc::Type> EasingFunc = EEasingFunc::EaseInOut);

	virtual void Activate() override;
};