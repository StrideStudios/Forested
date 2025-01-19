#include "CullStaticMeshComponent.h"

bool UCullStaticMeshComponent::SetStaticMesh(UStaticMesh* NewMesh) {
	const bool Success = Super::SetStaticMesh(NewMesh);
	RefreshBounds();
	return Success;
}

void UCullStaticMeshComponent::RefreshBounds() {
	//if a child of attach parent we should notify cull distance being set
	//this will follow chain until uppermost component
	if (GetAttachParent() && GetAttachParent()->IsA(StaticClass())) {
		CastChecked<UCullStaticMeshComponent>(GetAttachParent())->RefreshBounds();
		return;
	}
	RefreshBounds_Internal();
}

void UCullStaticMeshComponent::RefreshBounds_Internal() {
	if (!GetStaticMesh()) return;
	Bounds = GetStaticMesh()->GetBounds().TransformBy(GetRelativeTransform());
	ForEachChildComponent([this](const UCullStaticMeshComponent* Component) {
		if (!Component->GetStaticMesh()) return;
		Bounds = Bounds + Component->GetStaticMesh()->GetBounds().TransformBy(Component->GetRelativeTransform());
	});
	SetStaticMeshCullDistance(Bounds.SphereRadius * CullScale);
}


void UCullStaticMeshComponent::SetStaticMeshCullDistance(float CullDistance) {
	SetCullDistance(CullDistance);
	FString Names = GetName();
	ForEachChildComponent([&, CullDistance](UCullStaticMeshComponent* Component) {
		Component->SetCullDistance(CullDistance);
		Names += Component->GetName();
	});
}
