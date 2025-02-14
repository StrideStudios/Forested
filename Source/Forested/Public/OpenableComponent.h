#pragma once

#include "Forested/ForestedMinimal.h"
#include "SelectableInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "OpenableComponent.generated.h"

class AWaterActor;
class AFPlayer;
class UInputMappingContext;

UCLASS(ClassGroup = Utility, Blueprintable, meta=(BlueprintSpawnableComponent, PrioritizeCategories = "Light Ease Movement"))
class FORESTED_API UOpenableComponent : public USceneComponent {
	GENERATED_BODY()

public:

	UOpenableComponent();

protected:

	virtual void BeginPlay() override;

public:

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	void Open();

	void Close();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Light")
	float GetTime() const { return Time; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Light")
	float GetTargetTime() const { return TargetTime; }

	//the interpolation type when opening
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ease")
	TEnumAsByte<EEasingFunc::Type> OpenInterp = EEasingFunc::EaseIn;

	//the interpolation type when closing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ease")
	TEnumAsByte<EEasingFunc::Type> CloseInterp = EEasingFunc::EaseOut;

	//whether it should bounce or overshoot when opening
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ease")
	bool bBounceOpen = false;

	//whether it should bounce or overshoot when closing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ease")
	bool bBounceClose = false;

	//how large the bounce is
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ease")
	float Amplitude = 1.f;

	//how quickly the bounce slows down
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ease")
	float Decay = 5.f;

	//the frequency of the bounce
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ease")
	float Frequency = 1.f;

	//the movement speed of the component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ease")
	float Speed = 1.f;

	//the exponent used when easing to target
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ease")
	float BlendExp = 1.5f;

	//whether the component is open or not
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bOpen = false;

	//the cutoff for time multiplied by speed, this means the actor will open/close for 2.5 seconds if speed is 1
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float TimeThreshold = 2.5f;

	//the transform used when the actor is open
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	FTransform OpenTransform = FTransform::Identity;

	//the transform used when the actor is closed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	FTransform CloseTransform = FTransform::Identity;

private:

	float Time = 0.f, TargetTime = 0.f;
	
};

UCLASS(Blueprintable)
class FORESTED_API AOpenableActor : public AActor {
	GENERATED_BODY()

public:

	AOpenableActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* Root;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UOpenableComponent* OpenableComponent;
	
};

UCLASS(Blueprintable, meta=(PrioritizeCategories = "Selectable"))
class FORESTED_API ASelectableOpenableActor : public AOpenableActor, public ISelectableInterface {
	GENERATED_BODY()

public:

	ASelectableOpenableActor();

	virtual bool CanSelect_Implementation(const AFPlayer* Player, const FHitResult& HitResult, const float HeldTime) const override;

	virtual bool Selected_Implementation(AFPlayer* Player, const FHitResult& HitResult, const float HeldTime) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selectable")
	bool bIsSelectable = true;
	
};