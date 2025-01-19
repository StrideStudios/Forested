#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "CullStaticMeshComponent.generated.h"

/*
 * A class which automatically sets a cull distance based on a few parameters
 * this cull mesh and ALL child cull meshes will be included
 * the distance is scaled by cull scale
 */
UCLASS(Blueprintable, ClassGroup = (Rendering, Common), editinlinenew, meta = (BlueprintSpawnableComponent, PrioritizeCategories = "Culling"))
class FORESTED_API UCullStaticMeshComponent : public UStaticMeshComponent {
	GENERATED_BODY()

protected:

	virtual void BeginPlay() override {
		Super::BeginPlay();
		//should only be done if parent is not the same class
		if (GetAttachParent() && GetAttachParent()->IsA(StaticClass())) return;
		RefreshBounds_Internal();
	}
	
public:

	UCullStaticMeshComponent() {
		SetGenerateOverlapEvents(false);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Cull Static Mesh Component|Culling")
	FBoxSphereBounds GetBounds() const { return Bounds; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Cull Static Mesh Component|Culling")
	float GetCullScale() const { return CullScale; }

	UFUNCTION(BlueprintCallable, Category = "Cull Static Mesh Component|Culling")
	void SetCullScale(const float InCullScale) {
		CullScale = InCullScale;
		SetStaticMeshCullDistance(Bounds.SphereRadius * CullScale);
	}
	
	virtual bool SetStaticMesh(UStaticMesh* NewMesh) override;

	UFUNCTION(BlueprintCallable, Category = "Cull Static Mesh Component|Culling")
	virtual void RefreshBounds();

	UFUNCTION(BlueprintCallable, Category = "Cull Static Mesh Component|Culling")
	void SetStaticMeshCullDistance(float CullDistance);
	
	void ForEachChildComponent(const TFunction<void(UCullStaticMeshComponent*)>& Function) const {
		TArray<USceneComponent*> SceneComponents = GetAttachChildren();
		SceneComponents.Sort([](const USceneComponent& S1, const USceneComponent& S2) {
			return S1.GetName() > S2.GetName();
		});
		for (USceneComponent* Component : SceneComponents) {
			if (!Component) continue;
			if (UCullStaticMeshComponent* CullComponent = Cast<UCullStaticMeshComponent>(Component)) {
				Function(CullComponent);
				CullComponent->ForEachChildComponent(Function);
			}
		}
	}


private:

	void RefreshBounds_Internal();

	FBoxSphereBounds Bounds;

	//multiplies the calculated cull distance by this number
	UPROPERTY(EditDefaultsOnly, Category="Culling")
	float CullScale  = 25.f;
	
};