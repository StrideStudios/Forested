#include "Lights/OpenableEnvironmentLight.h"
#include "Components/OpenableComponent.h"
#include "Components/LightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/RectLightComponent.h"

template <typename Pred>
void Refresh(ULightComponent* LightComponent, AOpenableActor* OpenableActor, const bool bIsActiveOnOpen, Pred&& RefreshParent) {
	if (OpenableActor) {

		//if in editor world set intensity to final result
#if WITH_EDITOR
	if (TEST_PLAYING(OpenableActor->GetWorld(), EWorldType::Editor)) {
		RefreshParent();
		LightComponent->SetVisibility(OpenableActor->OpenableComponent->bOpen == bIsActiveOnOpen);
		return;
	}
#endif

		//adjust intensity to match the openable component
		const float Time = FMath::Clamp(OpenableActor->OpenableComponent->GetTime(), 0.f, 1.f);
		const float AdjTime = bIsActiveOnOpen ? 1.f - Time : Time;
		if (AdjTime > 0.f) {
			RefreshParent();
			const float NewIntensity = AdjTime * LightComponent->Intensity;
			LightComponent->SetIntensity(NewIntensity);
			LightComponent->SetVisibility(NewIntensity > 0.f);
		}
		return;
	}
	LightComponent->SetVisibility(false);
}

void AOpenableEnvironmentPointLight::RefreshLight(float DeltaTime, const ASky* Sky) {
	if (!IsEnabled()) {
		Super::RefreshLight(DeltaTime, Sky);
		return;
	}
	if (!Sky) return;
	Refresh(PointLight, OpenableActor, bActiveOnOpen, [this, DeltaTime, Sky] {
		Super::RefreshLight(DeltaTime, Sky);
	});
}

void AOpenableEnvironmentRectLight::RefreshLight(float DeltaTime, const ASky* Sky) {
	if (!IsEnabled()) {
		Super::RefreshLight(DeltaTime, Sky);
		return;
	}
	if (!Sky) return;
	Refresh(RectLight, OpenableActor, bActiveOnOpen, [this, DeltaTime, Sky] {
		Super::RefreshLight(DeltaTime, Sky);
	});
}

void AOpenableEnvironmentBounceLight::RefreshLight(float DeltaTime, const ASky* Sky) {
	if (!IsEnabled()) {
		Super::RefreshLight(DeltaTime, Sky);
		return;
	}
	if (!Sky) return;
	Refresh(BounceLight, OpenableActor, bActiveOnOpen, [this, DeltaTime, Sky] {
		Super::RefreshLight(DeltaTime, Sky);
	});
}