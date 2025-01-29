#pragma once

#include "CoreMinimal.h"
#include "TreeChildComponent.h"
#include "Sky.h"
#include "GameFramework/Actor.h"
#include "TreeChildActor.generated.h"

class UCapsuleComponent;
class AFPlayer;

UCLASS(BlueprintType, meta = (PrioritizeCategories = "Growth Damage Mesh"))
class FORESTED_API ATreeChildActor : public AActor {
	GENERATED_BODY()
	
public:	
	ATreeChildActor();

protected:
	virtual void BeginPlay() override;

public:

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tree Child Actor")
	FORCEINLINE int GetHealth() const { return Health; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tree Child Actor")
	FORCEINLINE TArray<UTreeChildComponent*> GetTreeChildComponents() const { return TreeChildComponents; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tree Child Actor")
	FORCEINLINE UStaticMesh* GetTreeInstanceMesh() const { return TreeInstanceMesh; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tree Child Actor")
	FORCEINLINE UStaticMesh* GetStumpMesh() const { return StumpMesh; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tree Child Actor")
	FORCEINLINE TSubclassOf<UItem> GetLogItemClass() const { return LogItemClass; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tree Child Actor")
	FORCEINLINE float GetStartGrowthTime() const { return StartGrowthTime; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tree Child Actor")
	FORCEINLINE float GetEndGrowthTime() const { return EndGrowthTime; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tree Child Actor")
	FORCEINLINE float GetMinSize() const { return MinTreeSize; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tree Child Actor")
	FORCEINLINE FVector GetTreeFallDirection() const {
		return TreeFallDirection;
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tree Child Actor")
	FORCEINLINE bool IsFalling() const {
		return !TreeFallDirection.IsZero();
	}

	virtual void Tick(float DeltaTime) override;

	bool DamageTree(AFPlayer* Player, const float Damage);

	void SetTreeFelled(const FVector& FallDirection);

	void Init();
	
	void LoadTick();
	
	void Load();

	void Unload();

	void OnGameSave();

	void OnGameLoad();

	void LoadDefaults();

	UFUNCTION()
	void OnTreeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTreeComponentOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tree Child Actor")
	float GetCurrentSize() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tree Child Actor")
	float GetCurrentGrowth() const;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	USceneComponent* Root;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	USceneComponent* TreeRoot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	USceneComponent* TreeMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	USceneComponent* RotatePoint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	UCapsuleComponent* BranchCollision;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	UCapsuleComponent* TrunkCollision;

protected:

	UPROPERTY(SaveGame)
	float Health = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Mesh|Defaults")
	UStaticMesh* TreeInstanceMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Mesh|Defaults")
	UStaticMesh* StumpMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TSubclassOf<UItem> LogItemClass;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float FallSpeed = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float FallPower = 2.f;

	UPROPERTY(EditDefaultsOnly, Category = "Growth", meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
	float StartGrowth = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Growth", meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
	float CanDamageGrowth = 50.f;

	UPROPERTY(EditDefaultsOnly, Category = "Growth", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DaysUntilGrown = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Growth")
	float MinTreeSize = 0.05f;
	
private:

	UPROPERTY()
	TArray<UTreeChildComponent*> TreeChildComponents;

	UPROPERTY(SaveGame)
	TMap<uint8, FObjectData> TreeChildData;
	
	UPROPERTY(SaveGame)
	float StartGrowthTime;

	UPROPERTY(SaveGame)
	float EndGrowthTime;
	
	UPROPERTY(SaveGame)
	FVector TreeFallDirection = FVector::ZeroVector;

	float FelledTime;
};
