#pragma once

#include "Forested/ForestedMinimal.h"
#include "EnvironmentLight.h"
#include "OpenableEnvironmentLight.generated.h"

class AOpenableActor;
class UArrowComponent;
class ASky;
class UPointLightComponent;
class URectLightComponent;

UCLASS(meta=(PrioritizeCategories = "Openable"))
class AOpenableEnvironmentPointLight : public AEnvironmentPointLight {
	GENERATED_BODY()

public:
	
	virtual void RefreshLight(float DeltaTime, const ASky* Sky) override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Openable")
	AOpenableActor* OpenableActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Openable")
	bool bActiveOnOpen = true;
	
};

UCLASS(meta=(PrioritizeCategories = "Openable"))
class AOpenableEnvironmentRectLight : public AEnvironmentRectLight {
	GENERATED_BODY()

public:
	
	virtual void RefreshLight(float DeltaTime, const ASky* Sky) override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Openable")
	AOpenableActor* OpenableActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Openable")
	bool bActiveOnOpen = true;
	
};

UCLASS(meta=(PrioritizeCategories = "Openable"))
class AOpenableEnvironmentBounceLight : public AEnvironmentBounceLight {
	GENERATED_BODY()

public:
	
	virtual void RefreshLight(float DeltaTime, const ASky* Sky) override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Openable")
	AOpenableActor* OpenableActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Openable")
	bool bActiveOnOpen = true;
	
};