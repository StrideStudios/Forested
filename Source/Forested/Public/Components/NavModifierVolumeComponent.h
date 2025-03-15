#pragma once

#include "Forested/ForestedMinimal.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "NavModifierVolumeComponent.generated.h"

UCLASS(ClassGroup="Collision", hidecategories=(Object,LOD,Lighting,TextureStreaming), editinlinenew, meta=(DisplayName="Nav Modifier Box Collision", BlueprintSpawnableComponent))
class FORESTED_API UNavModifierBoxComponent : public UBoxComponent {
	GENERATED_BODY()
	
	virtual bool IsNavigationRelevant() const override {
		if (!CanEverAffectNavigation()) {
			return false;
		}
		
		return true;
	}
};

UCLASS(ClassGroup="Collision", hidecategories=(Object,LOD,Lighting,TextureStreaming), editinlinenew, meta=(DisplayName="Nav Modifier Sphere Collision", BlueprintSpawnableComponent))
class FORESTED_API UNavModifierSphereComponent : public USphereComponent {
	GENERATED_BODY()
	
	virtual bool IsNavigationRelevant() const override {
		if (!CanEverAffectNavigation()) {
			return false;
		}
		
		return true;
	}
};

UCLASS(ClassGroup="Collision", hidecategories=(Object,LOD,Lighting,TextureStreaming), editinlinenew, meta=(DisplayName="Nav Modifier Capsule Collision", BlueprintSpawnableComponent))
class FORESTED_API UNavModifierCapsuleComponent : public UCapsuleComponent {
	GENERATED_BODY()
	
	virtual bool IsNavigationRelevant() const override {
		if (!CanEverAffectNavigation()) {
			return false;
		}
		
		return true;
	}
};