#include "OpenableComponent.h"
#include "ForestedLibrary.h"

UOpenableComponent::UOpenableComponent() {
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UOpenableComponent::BeginPlay() {
	Super::BeginPlay();
	
	SetRelativeTransform(bOpen ? OpenTransform : CloseTransform);
	Time = 1.f - bOpen;
}

void UOpenableComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Time += DeltaTime * (bOpen ? -Speed : Speed);
	const float EaseValue = UForestedLibrary::Bounce(Time, bOpen, bOpen ? bBounceOpen : bBounceClose, 0.f, Amplitude, Decay, Frequency, CloseInterp, OpenInterp, BlendExp, 2);
	SetRelativeTransform(UKismetMathLibrary::TLerp(OpenTransform, CloseTransform, EaseValue));

	//if the time is past a certain threshold, the bounce shouldn't be noticeable
	if (Time <= -TimeThreshold + 1.f || Time >= TimeThreshold) {
		SetComponentTickEnabled(false);
	}
}

void UOpenableComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : FName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UOpenableComponent, bOpen)) {
		SetRelativeTransform(bOpen ? OpenTransform : CloseTransform);
	}
}

void UOpenableComponent::Open() {
	bOpen = true;
	Time = FMath::Clamp(Time, 0.f, 1.f);
	SetComponentTickEnabled(true);
}

void UOpenableComponent::Close() {
	bOpen = false;
	Time = FMath::Clamp(Time, 0.f, 1.f);
	SetComponentTickEnabled(true);
}

AOpenableActor::AOpenableActor() {
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	OpenableComponent = CreateDefaultSubobject<UOpenableComponent>(TEXT("Openable Component"));
	OpenableComponent->SetupAttachment(GetRootComponent());
}

ASelectableOpenableActor::ASelectableOpenableActor() {
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

bool ASelectableOpenableActor::CanSelect_Implementation(const AFPlayer* Player, const FHitResult& HitResult, const float HeldTime) const {
	return bIsSelectable;
}

bool ASelectableOpenableActor::Selected_Implementation(AFPlayer* Player, const FHitResult& HitResult, const float HeldTime) {
	if (OpenableComponent->bOpen) {
		OpenableComponent->Close();
		return true;
	}
	OpenableComponent->Open();
	return true;
}
