#pragma once

#include "Forested/ForestedMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Animation/AnimInstance.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "MontageLibrary.generated.h"

struct FAnimMontageInstance;
struct FAlphaBlend;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMontageStartDelegate, FName, NotifyName);

DECLARE_DELEGATE_OneParam(FOnMontageNotify, FName) 
DECLARE_DELEGATE_TwoParams(FOnMontageEnded, UAnimMontage*, bool) 
DECLARE_DELEGATE_TwoParams(FOnMontageBlendingOut, UAnimMontage*, bool) 

UCLASS()
class FORESTED_API UMontageLibrary : public UBlueprintFunctionLibrary {
	GENERATED_BODY()

public:
	
	static bool StartMontage(const USkeletalMeshComponent* SkeletalMeshComponent, UAnimMontage* Montage, float PlayRate = 1.f, float StartingPosition = 0.f, bool bCheckGroup = true, const TDelegateWrapper<FOnMontageEnded>& OnComplete = {}, const TDelegateWrapper<FOnMontageBlendingOut>& OnBlendOut = {}, const TDelegateWrapper<FOnMontageNotify>& OnNotifyBegin = {}, const TDelegateWrapper<FOnMontageNotify>& OnNotifyEnd = {});

	UFUNCTION(BlueprintCallable, Category = "Montage Library")
	static bool PauseMontage(USkeletalMeshComponent* SkeletalMeshComponent, const UAnimMontage* Montage); 

	UFUNCTION(BlueprintCallable, Category = "Montage Library")
	static bool ResumeMontage(USkeletalMeshComponent* SkeletalMeshComponent, const UAnimMontage* Montage); 

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = 2), Category = "Montage Library")
	static bool StopMontage(USkeletalMeshComponent* SkeletalMeshComponent, const UAnimMontage* Montage, float BlendOutTime); 
	
	static bool StopMontage(const USkeletalMeshComponent* SkeletalMeshComponent, const UAnimMontage* Montage, const FAlphaBlend& Blend);
	
	/**
	 * Montage Functions
	 */

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (AdvancedDisplay = 1), Category = "Montage Library")
	static UAnimMontage* GetCurrentActiveMontageOfGroup(const UAnimInstance* AnimInstance, FName GroupName);

	//returns true if any montage is active in a certain group
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (AdvancedDisplay = 2), Category = "Montage Library")
	static bool IsAMontageOfGroupActive(const UAnimInstance* AnimInstance, FName GroupName, bool IncludeBlendingOut = false);
	
	//returns true if Montage is active
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (AdvancedDisplay = 2), Category = "Montage Library")
	static bool IsMontageActive(const UAnimInstance* AnimInstance, const UAnimMontage* Montage, const bool IncludeBlendingOut = false);

	//returns true if Montage is active and playing
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (AdvancedDisplay = 2), Category = "Montage Library")
	static bool IsMontagePlaying(const UAnimInstance* AnimInstance, const UAnimMontage* Montage, const bool IncludeBlendingOut = false);

	//returns true if any Montage is active
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (AdvancedDisplay = 1), Category = "Inventory Render Actor|Montage")
	static bool IsAMontageActive(const UAnimInstance* AnimInstance, bool IncludeBlendingOut = false);

	//returns true if any Montage is active and playing
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (AdvancedDisplay = 1), Category = "Inventory Render Actor|Montage")
	static bool IsAMontagePlaying(const UAnimInstance* AnimInstance, bool IncludeBlendingOut = false);
	
	/**
	 * Montage Instance Functions
	 */

	//get the instance for a specified montage if it exists
	static FAnimMontageInstance* GetMontageInstance(UAnimInstance* AnimInstance, const UAnimMontage* Montage = nullptr, const bool IncludeBlendingOut = false);
	
	//returns a valid instance if any montage instance is active
	static FAnimMontageInstance* GetAnyActiveMontageInstance(UAnimInstance* AnimInstance, bool IncludeBlendingOut = false);

	//returns a valid instance if input montage is active and it exists
	static FAnimMontageInstance* GetActiveMontageInstance(const UAnimInstance* AnimInstance, const UAnimMontage* Montage, const bool IncludeBlendingOut = false);
	
	//returns true if Montage Instance is active
	static bool IsMontageInstanceActive(const FAnimMontageInstance* Montage, const bool IncludeBlendingOut = false);

	//returns true if Montage Instance is active and playing
	static bool IsMontageInstancePlaying(const FAnimMontageInstance* Montage, const bool IncludeBlendingOut = false);

private:
	
	static bool IsMontageActive_Internal(const FAnimMontageInstance* MontageInstance, const bool IncludeBlendingOut);
	
};

UCLASS()
class FORESTED_API UStartMontage : public UObject {
	GENERATED_BODY()

	friend class UMontageLibrary;
	friend class UStartMontageAsyncAction;

	/*
	 * parameters
	 */

	TDelegateWrapper<FOnMontageNotify> OnNotifyBegin;

	TDelegateWrapper<FOnMontageNotify> OnNotifyEnd;

	TDelegateWrapper<FOnMontageEnded> OnMontageEnded;

	TDelegateWrapper<FOnMontageBlendingOut> OnMontageBlendingOut;

	bool* OutSuccess;

	/*
	 * functions
	 */

	UFUNCTION()
	void OnNotifyBeginReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	UFUNCTION()
	void OnNotifyEndReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);
	
	bool StartMontage(const USkeletalMeshComponent* SkeletalMeshComponent, UAnimMontage* Montage, float PlayRate = 1.f, float StartingPosition = 0.f, bool bCheckGroup = true);
	
};

UCLASS()
class FORESTED_API UStartMontageAsyncAction : public UBlueprintAsyncActionBase {
	GENERATED_BODY()

protected:

	/*
	 * parameters
	 */

	bool* OutSuccess;

	USkeletalMeshComponent* SkeletalMeshComponent;

	UAnimMontage* MontageToPlay;

	float Rate = 1.f;

	float StartTimeSeconds = 0.f;

	bool bCheckGroup = true;

	bool bInterruptedCalledBeforeBlendingOut = false;
	
	// Called when Montage finished playing and wasn't interrupted
	UPROPERTY(BlueprintAssignable)
	FOnMontageStartDelegate OnCompleted;

	// Called when Montage starts blending out and is not interrupted
	UPROPERTY(BlueprintAssignable)
	FOnMontageStartDelegate OnBlendOut;

	// Called when Montage has been interrupted (or failed to play)
	UPROPERTY(BlueprintAssignable)
	FOnMontageStartDelegate OnInterrupted;

	UPROPERTY(BlueprintAssignable)
	FOnMontageStartDelegate OnNotifyBegin;

	UPROPERTY(BlueprintAssignable)
	FOnMontageStartDelegate OnNotifyEnd;
	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Montage Library")
	static UStartMontageAsyncAction* StartMontage(bool& OutSuccess, USkeletalMeshComponent* SkeletalMeshComponent, UAnimMontage* Montage, bool bCheckGroup = true, float PlayRate = 1.f, float StartingPosition = 0.f);

	virtual void Activate() override;
	
};