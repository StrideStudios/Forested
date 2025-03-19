#pragma once

#include "Forested/ForestedMinimal.h"
#include "Engine/StreamableManager.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "SerializationLibrary.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAsyncPtrLoadComplete, UObject*, Object);

struct FSoftPtr {
	
	template <class TClass = UObject>
	FSoftPtr(const TSoftObjectPtr<TClass>& SoftObject):
	SoftObject(SoftObject),
	IsClass(false){
	}

	template <class TClass = UObject>
	FSoftPtr(const TSoftClassPtr<TClass>& SoftClass):
	SoftClass(SoftClass),
	IsClass(true) {
	}
	
	TSoftObjectPtr<> SoftObject = {};

	TSoftClassPtr<> SoftClass = {};

	FORCEINLINE const FSoftObjectPath& GetPath() const {
		return IsClass ? SoftClass.ToSoftObjectPath() : SoftObject.ToSoftObjectPath();
	}

	//returns true if the object is loaded
	FORCEINLINE bool IsValid() const {
		return IsClass ? SoftClass.IsValid() : SoftObject.IsValid();
	}

	//returns true if object is null
	FORCEINLINE bool IsNull() const {
		return IsClass ? SoftClass.IsNull() : SoftObject.IsNull();
	}

private:
	
	bool IsClass;
	
};

class FSerializationLibrary {
	
public:

	static TSharedPtr<FStreamableHandle> LoadAsync(const FSoftPtr& TargetToStream, const TDelegateWrapper<FStreamableDelegate>& Delegate) {
		return LoadAsync(TArray<FSoftPtr>{TargetToStream}, Delegate);
	}

	static TSharedPtr<FStreamableHandle> LoadAsync(const TArray<FSoftPtr>& TargetsToStream, const TDelegateWrapper<FStreamableDelegate>& Delegate) {
		const TArray<FSoftObjectPath> Targets = GetInvalidTargets(TargetsToStream);
		if (!Targets.Num()) {
			Delegate.Execute();
			return {};
		}
		return LoadAsync_Internal(Targets, Delegate);
	}
	
	static TSharedPtr<FStreamableHandle> LoadSync(const FSoftPtr& TargetToStream) {
		return LoadSync(TArray<FSoftPtr>{TargetToStream});
	}

	static TSharedPtr<FStreamableHandle> LoadSync(const TArray<FSoftPtr>& TargetsToStream) {
		const TArray<FSoftObjectPath> Targets = GetInvalidTargets(TargetsToStream);
		if (!Targets.Num()) return {};
		return LoadSync_Internal(Targets);
	}

	static void Unload(const FSoftPtr& TargetToStream) {
		Unload(TArray<FSoftPtr>{TargetToStream});
	}

	static void Unload(const TArray<FSoftPtr>& TargetsToStream) {
		const TArray<FSoftObjectPath> Targets = GetValidTargets(TargetsToStream);
		if (!Targets.Num()) {
			return;
		}
		return Unload_Internal(Targets);
	}

private:

	static TArray<FSoftObjectPath> GetValidTargets(const TArray<FSoftPtr>& TargetsToStream) {
		TArray<FSoftObjectPath> Targets;
		for (const FSoftPtr Path : TargetsToStream) {
			//if null or not loaded we can ignore it
			if (Path.IsNull() || !Path.IsValid()) continue;
			Targets.Add(Path.GetPath());
		}
		return Targets;
	}

	static TArray<FSoftObjectPath> GetInvalidTargets(const TArray<FSoftPtr>& TargetsToStream) {
		TArray<FSoftObjectPath> Targets;
		for (const FSoftPtr Path : TargetsToStream) {
			//if null or already loaded we can ignore it
			if (Path.IsNull() || Path.IsValid()) continue;
			Targets.Add(Path.GetPath());
		}
		return Targets;
	}

	static TSharedPtr<FStreamableHandle> LoadSync_Internal(const TArray<FSoftObjectPath>& TargetsToStream);

	static TSharedPtr<FStreamableHandle> LoadAsync_Internal(const TArray<FSoftObjectPath>& TargetsToStream, const FStreamableDelegate& Delegate);

	static void Unload_Internal(const TArray<FSoftObjectPath>& TargetsToStream);
};

UCLASS()
class FORESTED_API ULoadSoftPtrAsyncAction : public UBlueprintAsyncActionBase {
	GENERATED_BODY()

protected:

	/*
	 * parameters
	 */
	
	TSoftObjectPtr<> SoftPtr;

	UPROPERTY(BlueprintAssignable)
	FOnAsyncPtrLoadComplete Complete;

	/*
	 * functions
	 */
	
	/**
	 * Loads an Object Ptr and calls complete
	 * @param SoftPtr the Ptr to load
	 */
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly = "true"), Category = "Serialization")
	static ULoadSoftPtrAsyncAction* LoadPtrAsync(const TSoftObjectPtr<UObject> SoftPtr);

	virtual void Activate() override;

};

//TODO: output actual object autocasted

UCLASS()
class FORESTED_API ULoadSoftClassPtrAsyncAction : public UBlueprintAsyncActionBase {
	GENERATED_BODY()

protected:

	/*
	 * parameters
	 */
	
	TSoftClassPtr<> SoftPtr;

	UPROPERTY(BlueprintAssignable)
	FOnAsyncPtrLoadComplete Complete;

	/*
	 * functions
	 */

	/**
	 * Loads a Class Ptr and calls complete
	 * @param SoftPtr the Ptr to load
	 */
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly = "true"), Category = "Serialization")
	static ULoadSoftClassPtrAsyncAction* LoadClassPtrAsync(const TSoftClassPtr<UObject> SoftPtr);

	virtual void Activate() override;

};