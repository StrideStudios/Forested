#include "Item/RenderActor.h"

ARenderActor::ARenderActor(const bool EnableCull):
EnableCull(EnableCull) {
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	SetupRootAttachment();
}

void ARenderActor::Init() {
	SetupRootAttachment();
	ReceiveInit();
	for (UActorComponent* Component : GetComponents()) {
		if (!Component || !Component->IsA(UPrimitiveComponent::StaticClass())) continue;
		PrimitiveComponents.Add(CastChecked<UPrimitiveComponent>(Component)); //will have to do with loading later on
	}
}

void ARenderActor::SetSimulatePhysics(const bool SimulatePhysics) {
	for (UPrimitiveComponent* Component : PrimitiveComponents) {
		Component->SetSimulatePhysics(SimulatePhysics);
	}
}

void ARenderActor::RefreshMeshBounds(const float CullDistanceOverride) {
	FBoxSphereBounds Bounds = PrimitiveComponents[0]->CalcLocalBounds();
	for (int i = 1; i < PrimitiveComponents.Num(); ++i) {
		if (const UPrimitiveComponent* Component = PrimitiveComponents[i])
			Bounds = Bounds + Component->CalcLocalBounds();
	}
	const float CullDistance = CullDistanceOverride ? CullDistanceOverride : Bounds.SphereRadius * 25.f;
	for (UPrimitiveComponent* Component : PrimitiveComponents) {
		if (!Component) continue;
		Component->SetCullDistance(CullDistance);
	}
}

void ARenderActor::SetupRootAttachment() const {
	if (AttachSocket == NAME_None) return;
	//if we are a child actor we need to attach that component
	UChildActorComponent* Component = GetParentComponent();
	if (Component && Component->GetAttachParent()) {
		if (USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(Component->GetAttachParent())) {
			Component->AttachToComponent(SkeletalMeshComponent, FAttachmentTransformRules::KeepRelativeTransform, SkeletalMeshComponent->DoesSocketExist(AttachSocket) ? AttachSocket : NAME_None);
			return;
		}
	}
	if (USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(Root->GetAttachParent())) {
		Root->AttachToComponent(SkeletalMeshComponent, FAttachmentTransformRules::KeepRelativeTransform, SkeletalMeshComponent->DoesSocketExist(AttachSocket) ? AttachSocket : NAME_None);
	}
}