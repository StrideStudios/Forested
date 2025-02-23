#include "Serialization/SerializationLibrary.h"
#include "Engine/AssetManager.h"

TSharedPtr<FStreamableHandle> FSerializationLibrary::LoadSync_Internal(const TArray<FSoftObjectPath>& TargetsToStream) {
	return UAssetManager::GetStreamableManager().RequestSyncLoad(TargetsToStream);
}

TSharedPtr<FStreamableHandle> FSerializationLibrary::LoadAsync_Internal(const TArray<FSoftObjectPath>& TargetsToStream, const FStreamableDelegate& Delegate) {
	return UAssetManager::GetStreamableManager().RequestAsyncLoad(TargetsToStream, Delegate);
}

void FSerializationLibrary::Unload_Internal(const TArray<FSoftObjectPath>& TargetsToStream) {
	for (const FSoftObjectPath& Path : TargetsToStream) {
		UAssetManager::GetStreamableManager().Unload(Path);
	}
}

ULoadSoftPtrAsyncAction* ULoadSoftPtrAsyncAction::LoadPtrAsync(const TSoftObjectPtr<> SoftPtr) {
	if (SoftPtr.IsNull()) {
		LOG_ERROR("Given Soft Ptr was null");
		return nullptr;
	}
	ULoadSoftPtrAsyncAction* Node = NewObject<ULoadSoftPtrAsyncAction>();
	Node->SoftPtr = SoftPtr;
	return Node;
}

void ULoadSoftPtrAsyncAction::Activate() {
	FSerializationLibrary::LoadAsync(SoftPtr, [this] {
		Complete.Broadcast(SoftPtr.Get());
	});
}

ULoadSoftClassPtrAsyncAction* ULoadSoftClassPtrAsyncAction::LoadClassPtrAsync(const TSoftClassPtr<> SoftPtr) {
	if (SoftPtr.IsNull()) {
		LOG_ERROR("Given Soft Class Ptr was null");
		return nullptr;
	}
	ULoadSoftClassPtrAsyncAction* Node = NewObject<ULoadSoftClassPtrAsyncAction>();
	Node->SoftPtr = SoftPtr;
	return Node;
}

void ULoadSoftClassPtrAsyncAction::Activate() {
	FSerializationLibrary::LoadAsync(SoftPtr, [this] {
		Complete.Broadcast(SoftPtr.Get());
	});
}

