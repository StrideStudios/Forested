#include "MontageLibrary.h"

#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"

bool UMontageLibrary::StartMontage(const USkeletalMeshComponent* SkeletalMeshComponent, UAnimMontage* Montage, const float PlayRate, const float StartingPosition, const bool bCheckGroup, const TDelegateWrapper<FOnMontageEnded>& OnComplete, const TDelegateWrapper<FOnMontageBlendingOut>& OnBlendOut, const TDelegateWrapper<FOnMontageNotify>& OnNotifyBegin, const TDelegateWrapper<FOnMontageNotify>& OnNotifyEnd) {
	UStartMontage* StartMontage = NewObject<UStartMontage>();

	StartMontage->OnMontageEnded = OnComplete;
	StartMontage->OnMontageBlendingOut = OnBlendOut;
	StartMontage->OnNotifyBegin = OnNotifyBegin;
	StartMontage->OnNotifyEnd = OnNotifyEnd;
	
	return StartMontage->StartMontage(SkeletalMeshComponent, Montage, PlayRate, StartingPosition, bCheckGroup);
}

bool UMontageLibrary::PauseMontage(USkeletalMeshComponent* SkeletalMeshComponent, const UAnimMontage* Montage) {
	if (!SkeletalMeshComponent || !SkeletalMeshComponent->GetAnimInstance()) return false;
	UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
	FAnimMontageInstance* ActiveMontage = GetMontageInstance(AnimInstance, Montage, true); 
	if (ActiveMontage && IsMontageActive(AnimInstance, Montage)) {
		ActiveMontage->Pause();
		return true;
	}

	return false;
}

bool UMontageLibrary::ResumeMontage(USkeletalMeshComponent* SkeletalMeshComponent, const UAnimMontage* Montage) {
	if (!SkeletalMeshComponent || !SkeletalMeshComponent->GetAnimInstance()) return false;
	UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
	FAnimMontageInstance* ActiveMontage = GetMontageInstance(AnimInstance, Montage, true); 
	if (ActiveMontage && !ActiveMontage->IsPlaying() && IsMontageActive(AnimInstance, Montage) && !IsMontagePlaying(AnimInstance, Montage)) {
		ActiveMontage->SetPlaying(true);
		return true;
	}
	return false;
}

bool UMontageLibrary::StopMontage(USkeletalMeshComponent* SkeletalMeshComponent, const UAnimMontage* Montage, const float BlendOutTime) {
	if (!SkeletalMeshComponent || !SkeletalMeshComponent->GetAnimInstance()) return false;
	const UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
	if (!IsMontageActive(AnimInstance, Montage)) return false;
	FAlphaBlend Blend = FAlphaBlend(Montage->BlendOut);
	Blend.SetBlendTime(BlendOutTime);
	return StopMontage(SkeletalMeshComponent, Montage, Blend);
}

bool UMontageLibrary::StopMontage(const USkeletalMeshComponent* SkeletalMeshComponent, const UAnimMontage* Montage, const FAlphaBlend& Blend) {
	if (!SkeletalMeshComponent || !SkeletalMeshComponent->GetAnimInstance()) return false;
	UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
	if (FAnimMontageInstance* ActiveMontage = GetMontageInstance(AnimInstance, Montage, true)) {
		ActiveMontage->Stop(Blend);
		return true;
	}
	return false;
}

UAnimMontage* UMontageLibrary::GetCurrentActiveMontageOfGroup(const UAnimInstance* AnimInstance, const FName GroupName) {
	// Start from end, as most recent instances are added at the end of the queue.
	int32 const NumInstances = AnimInstance->MontageInstances.Num();
	for (int32 InstanceIndex = NumInstances - 1; InstanceIndex >= 0; InstanceIndex--) {
		const FAnimMontageInstance* MontageInstance = AnimInstance->MontageInstances[InstanceIndex];
		if (MontageInstance && MontageInstance->Montage && MontageInstance->Montage->GetGroupName() == GroupName && MontageInstance->IsActive()) {
			return MontageInstance->Montage;
		}
	}

	return nullptr;
}

bool UMontageLibrary::IsAMontageOfGroupActive(const UAnimInstance* AnimInstance, const FName GroupName, const bool IncludeBlendingOut) {
	for (const FAnimMontageInstance* MontageInstance : AnimInstance->MontageInstances) {
		if (MontageInstance && MontageInstance->Montage && MontageInstance->Montage->GetGroupName() == GroupName && IsMontageInstanceActive(MontageInstance, IncludeBlendingOut))
			return true;
	}
	return false;
}

bool UMontageLibrary::IsMontageActive(const UAnimInstance* AnimInstance, const UAnimMontage* Montage, const bool IncludeBlendingOut)  {
	return IsMontageInstanceActive(AnimInstance->GetActiveInstanceForMontage(Montage), IncludeBlendingOut);
}

bool UMontageLibrary::IsMontagePlaying(const UAnimInstance* AnimInstance, const UAnimMontage* Montage, const bool IncludeBlendingOut) {
	return IsMontageInstancePlaying(AnimInstance->GetActiveInstanceForMontage(Montage), IncludeBlendingOut);
}

bool UMontageLibrary::IsAMontageActive(const UAnimInstance* AnimInstance, const bool IncludeBlendingOut) {
	for (const FAnimMontageInstance* MontageInstance : AnimInstance->MontageInstances) {
		if (MontageInstance && IsMontageInstanceActive(MontageInstance, IncludeBlendingOut))
			return true;
	}
	return false;
}

bool UMontageLibrary::IsAMontagePlaying(const UAnimInstance* AnimInstance, const bool IncludeBlendingOut) {
	for (const FAnimMontageInstance* MontageInstance : AnimInstance->MontageInstances) {
		if (MontageInstance && IsMontageInstancePlaying(MontageInstance, IncludeBlendingOut))
			return true;
	}
	return false;
}

FAnimMontageInstance* UMontageLibrary::GetMontageInstance(UAnimInstance* AnimInstance, const UAnimMontage* Montage, const bool IncludeBlendingOut) {
	if (!Montage) return GetAnyActiveMontageInstance(AnimInstance, IncludeBlendingOut);
	return GetActiveMontageInstance(AnimInstance, Montage, IncludeBlendingOut);
}

FAnimMontageInstance* UMontageLibrary::GetAnyActiveMontageInstance(UAnimInstance* AnimInstance, const bool IncludeBlendingOut) {
	for (FAnimMontageInstance* MontageInstance : AnimInstance->MontageInstances) {
		if (MontageInstance && IsMontageInstanceActive(MontageInstance, IncludeBlendingOut))
			return MontageInstance;
	}
	return nullptr;
}

FAnimMontageInstance* UMontageLibrary::GetActiveMontageInstance(const UAnimInstance* AnimInstance, const UAnimMontage* Montage, const bool IncludeBlendingOut) {
	FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(Montage);
	if (MontageInstance && IsMontageInstanceActive(MontageInstance, IncludeBlendingOut))
		return MontageInstance;
	return nullptr;
}

bool UMontageLibrary::IsMontageInstanceActive(const FAnimMontageInstance* Montage, const bool IncludeBlendingOut) {
	return Montage && Montage->Montage && IsMontageActive_Internal(Montage, IncludeBlendingOut) && Montage->IsActive();
}

bool UMontageLibrary::IsMontageInstancePlaying(const FAnimMontageInstance* Montage, const bool IncludeBlendingOut) {
	return Montage && Montage->Montage && IsMontageActive_Internal(Montage, IncludeBlendingOut) && Montage->IsPlaying() && Montage->GetPosition() != Montage->GetPreviousPosition();
}

bool UMontageLibrary::IsMontageActive_Internal(const FAnimMontageInstance* MontageInstance, const bool IncludeBlendingOut) {
	return IncludeBlendingOut || !MontageInstance->bEnableAutoBlendOut || MontageInstance->GetPosition() < MontageInstance->Montage->GetPlayLength() - MontageInstance->Montage->BlendOut.GetBlendTime();
}

void UStartMontage::OnNotifyBeginReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload) {
	if (OnNotifyBegin.IsBound())
		OnNotifyBegin.ExecuteIfBound(NotifyName);
}

void UStartMontage::OnNotifyEndReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload) {
	if (OnNotifyEnd.IsBound())
		OnNotifyEnd.ExecuteIfBound(NotifyName);
}

bool UStartMontage::StartMontage(const USkeletalMeshComponent* SkeletalMeshComponent, UAnimMontage* Montage, const float PlayRate, const float StartingPosition, const bool bCheckGroup) {
	if (!SkeletalMeshComponent) {
		LOG_ERROR("Start Montage must have a valid Skeletal Mesh");
		return false;
	}
	
	if (!SkeletalMeshComponent->GetAnimInstance()) {
		LOG_ERROR("Start Montage must have a valid Skeletal Mesh Anim Instance");
		return false;
	}
	
	if (!Montage) {
		LOG_ERROR("Start Montage must have a valid Montage");
		return false;
	}

	const FName SlotName = Montage->GetGroupName();
	if (bCheckGroup && UMontageLibrary::IsAMontageOfGroupActive(SkeletalMeshComponent->GetAnimInstance(), SlotName)) {
		LOG_EDITOR_WARNING("Anim Instance %s has active montage %s in group %s.", *SkeletalMeshComponent->GetAnimInstance()->GetName(), *UMontageLibrary::GetCurrentActiveMontageOfGroup(SkeletalMeshComponent->GetAnimInstance(), SlotName)->GetName(), *SlotName.ToString());
		return false;
	}

	UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
	const float f = AnimInstance->Montage_Play(Montage, PlayRate, EMontagePlayReturnType::MontageLength, StartingPosition);
	if (f > 0.f) {
		AnimInstance->Montage_SetEndDelegate(OnMontageEnded, Montage);
		AnimInstance->Montage_SetBlendingOutDelegate(OnMontageBlendingOut, Montage);
		AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UStartMontage::OnNotifyBeginReceived);
		AnimInstance->OnPlayMontageNotifyEnd.AddDynamic(this, &UStartMontage::OnNotifyEndReceived);
	} else {
		LOG_EDITOR_WARNING("Unknown Montage Failure.");
	}
	
	return f > 0.f;
}

UStartMontageAsyncAction* UStartMontageAsyncAction::StartMontage(bool& OutSuccess, USkeletalMeshComponent* SkeletalMeshComponent, UAnimMontage* Montage, const bool bCheckGroup, const float PlayRate, const float StartingPosition) {
	UStartMontageAsyncAction* Node = NewObject<UStartMontageAsyncAction>();
	Node->OutSuccess = &OutSuccess;
	Node->SkeletalMeshComponent = SkeletalMeshComponent;
	Node->MontageToPlay = Montage;
	Node->Rate = PlayRate;
	Node->StartTimeSeconds = StartingPosition;
	Node->bCheckGroup = bCheckGroup;
	return Node;
}

void UStartMontageAsyncAction::Activate() {
	UStartMontage* StartMontage = NewObject<UStartMontage>();

	StartMontage->OnMontageEnded = [this](UAnimMontage* Montage, const bool bInterrupted) {
		if (!bInterrupted) {
			OnCompleted.Broadcast(NAME_None);
		} else if (!bInterruptedCalledBeforeBlendingOut) {
			OnInterrupted.Broadcast(NAME_None);
		}
	};
	StartMontage->OnMontageBlendingOut = [this](UAnimMontage* Montage, const bool bInterrupted) {
		if (bInterrupted) {
			OnInterrupted.Broadcast(NAME_None);
			bInterruptedCalledBeforeBlendingOut = true;
		} else {
			OnBlendOut.Broadcast(NAME_None);
		}
	};
	StartMontage->OnNotifyBegin = [this](const FName NotifyName) {
		OnNotifyBegin.Broadcast(NotifyName);
	};
	StartMontage->OnNotifyEnd = [this](const FName NotifyName) {
		OnNotifyEnd.Broadcast(NotifyName);
	};
	
	*OutSuccess = StartMontage->StartMontage(SkeletalMeshComponent, MontageToPlay, Rate, StartTimeSeconds, bCheckGroup);
}
