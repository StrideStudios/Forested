#include "Player/FPlayer.h"
#include "EngineUtils.h"
#include "Player/PlayerInventory.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h" 
#include "Kismet/GameplayStatics.h"
#include "Interfaces/LoadableInterface.h"
#include "Player/PlayerInputComponent.h"
#include "Widget/PlayerWidget.h"
#include "Interfaces/SelectableInterface.h"
#include "Serialization/SerializationLibrary.h"
#include "Sky.h"
#include "Player/ViewmodelMeshes.h"
#include "Components/SphereComponent.h"
#include "Player/PlayerHud.h"

AFPlayer::AFPlayer(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer
                                                                       .SetDefaultSubobjectClass<UPlayerMovementComponent>(CharacterMovementComponentName)
                                                                       .SetDefaultSubobjectClass<UViewmodelSkeletalMeshComponent>(MeshComponentName)) {
	
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	GetMesh()->bSelfShadowOnly = true;
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, BaseEyeHeight));
	CrouchedEyeHeight = GetCharacterMovement()->CrouchedHalfHeight * 0.80f;
	
	LoadMesh = CreateDefaultSubobject<USphereComponent>(TEXT("Load Mesh"));
	LoadMesh->SetCollisionObjectType(LOADABLE_OBJECT_CHANNEL);
	LoadMesh->SetCollisionResponseToAllChannels(ECR_Overlap);
	LoadMesh->SetupAttachment(GetCapsuleComponent());
	LoadMesh->SetSphereRadius(1000.f);
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetRelativeRotation(FRotator(0.f,-90.f,0.f));
	Camera->SetRelativeScale3D(FVector(0.01f));
	Camera->SetupAttachment(GetMesh(), "Camera");
	Camera->bUsePawnControlRotation = false;
	ItemMesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("Main Item Mesh"));
	ItemMesh->SetupAttachment(GetMesh());
	ItemMesh->SetVisibility(true);
	
	BreathingPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Breathing Point"));
	BreathingPoint->SetupAttachment(GetCapsuleComponent());
	BreathingPoint->SetRelativeLocation(Camera->GetRelativeLocation() + FVector(0.0, 0.0, 15.0));
	WaistPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Waist Point"));
	WaistPoint->SetupAttachment(GetCapsuleComponent());
	WaistPoint->SetRelativeLocation(Camera->GetRelativeLocation() - FVector(0.0, 0.0, 15.0));

	PlayerHudComponent = CreateDefaultSubobject<UPlayerHud>(TEXT("Player Hud Component"));
	PlayerInputComponent = CreateDefaultSubobject<UPlayerInputComponent>(TEXT("Player Input Component"));
	PlayerInventory = CreateDefaultSubobject<UPlayerInventory>(TEXT("Inventory"));
}

void AFPlayer::BeginPlay() {
	Super::BeginPlay();

	//make sure we attach the camera
	Camera->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, "Camera");

	//load widget class and create widget
	if (!PlayerHudClass.IsNull()) {
		FSerializationLibrary::LoadSync(PlayerHudClass);
		PlayerHud = CreateWidget<UPlayerHud>(GetPlayerController(), PlayerHudClass.Get());
		PlayerHud->SetMenuHud();

		PlayerHud->AddToViewport();
	}
	
	//add on destroy listeners for all current actors in the world
	for (TActorIterator<AActor> It(GetWorld()); It; ++It) {
		AddOnDestroyedListener(*It);
	}

	//each actor spawned will have an on destroyed listener added
	GetWorld()->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateUObject(this, &AFPlayer::OnActorSpawned));
	
	LoadMesh->OnComponentBeginOverlap.AddUniqueDynamic(this, &AFPlayer::LoadMeshOverlap);
	LoadMesh->OnComponentEndOverlap.AddUniqueDynamic(this, &AFPlayer::LoadMeshEndOverlap);
}

void AFPlayer::EnablePlayerMovement() const {
	if (!PlayerInputComponent) return;
	PlayerInputComponent->EnableSprint();
	PlayerInputComponent->EnableJump();
}

void AFPlayer::DisablePlayerMovement() const {
	PlayerInputComponent->DisableSprint();
	PlayerInputComponent->DisableJump();
}

//TODO: migrate to player Input Component
void AFPlayer::Tick(const float DeltaTime){
	Super::Tick(DeltaTime);
	for (AActor* Actor : LoadedTickingActors)
		ILoadableInterface::Execute_LoadTick(Actor);
	
	GetMesh()->SetWorldRotation(GetControlRotation());

	const FVector Start = Camera->GetComponentLocation();
	const FVector Direction = Camera->GetForwardVector();
	const FVector End = Direction * MaxHoverDistance + Start;

	FCollisionQueryParams Params(TEXT("LineTraceSingle"), SCENE_QUERY_STAT_ONLY(ForestedHoverTrace), true);
	Params.AddIgnoredActor(this);
	if (GetWorld()->LineTraceSingleByChannel(HoverHitResult, Start, End, HOVER_TRACE_CHANNEL, Params)) {
		AActor* ActorToSelect = HoverHitResult.GetActor();
		if (AActor* ParentActor = HoverHitResult.GetActor()->GetParentActor())
			ActorToSelect = ParentActor;
		if (ActorToSelect->GetClass()->ImplementsInterface(UHoverableInterface::StaticClass())) {
			if (IHoverableInterface::Execute_CanHover(ActorToSelect, this, HoverHitResult)) {
				IHoverableInterface::Execute_Hovered(ActorToSelect, this, HoverHitResult);
			}
		}
	}
}

UInputComponent* AFPlayer::CreatePlayerInputComponent() {
	return PlayerInputComponent;
}

APlayerController* AFPlayer::GetPlayerController() const {
	return CastChecked<APlayerController>(GetController());
}

bool AFPlayer::GetPlayerAnimInstance(UPlayerAnimInstance*& OutAnimInstance) const {
	if (!GetMesh() || !GetMesh()->GetAnimInstance()) return false;
	OutAnimInstance = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	return true;
}

bool AFPlayer::IsInputAllowed() const {
	return !GetWorld()->GetGameViewport()->IgnoreInput();
}

void AFPlayer::SetGameFocus() const {
	APlayerController* PlayerController = GetPlayerController();
	PlayerController->SetShowMouseCursor(false);
	FInputModeGameOnly InputMode;
	InputMode.SetConsumeCaptureMouseDown(true);
	PlayerController->SetInputMode(InputMode);
	PlayerController->FlushPressedKeys();
}

void AFPlayer::SetUIFocus(UWidget* InWidgetToFocus) const {
	APlayerController* PlayerController = GetPlayerController();
	PlayerController->SetShowMouseCursor(true);
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	if (InWidgetToFocus) {
		const TSharedPtr<SWidget> Widget = InWidgetToFocus->TakeWidget();
		if (!Widget->SupportsKeyboardFocus()) {
			LOG_ERROR("Widget %s does not support focus", *InWidgetToFocus->GetName());
		} else {
			InputMode.SetWidgetToFocus(Widget);
		}
	}
	PlayerController->SetInputMode(InputMode);
	PlayerController->FlushPressedKeys();
	ResetMousePosition();
}

void AFPlayer::SetGameAndUIFocus(UWidget* InWidgetToFocus) const {
	APlayerController* PlayerController = GetPlayerController();
	PlayerController->SetShowMouseCursor(true);
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	if (InWidgetToFocus) {
		const TSharedPtr<SWidget> Widget = InWidgetToFocus->TakeWidget();
		if (!Widget->SupportsKeyboardFocus()) {
			LOG_ERROR("Widget %s does not support focus", *InWidgetToFocus->GetName());
		} else {
			InputMode.SetWidgetToFocus(Widget);
		}
	}
	PlayerController->SetInputMode(InputMode);
	PlayerController->FlushPressedKeys();
	ResetMousePosition();
}

void AFPlayer::ResetMousePosition() const {
	APlayerController* PlayerController = GetPlayerController();
	int32 SizeX;
	int32 SizeY;
	PlayerController->GetViewportSize(SizeX, SizeY);
	PlayerController->SetMouseLocation(SizeX/2,SizeY/2);
}

void AFPlayer::InterpolatePlayerTo(const FTransform& Transform, const float Time, const TEnumAsByte<EEasingFunc::Type> EasingFunc, const TDelegateWrapper<FInterpolatePlayerDelegate>& OnUpdate, const TDelegateWrapper<FInterpolatePlayerDelegate>& OnComplete) {
	const FVector& OriginalLocation = GetActorLocation();
	const FRotator& OriginalRotation = GetControlRotation();
	SKY->AddTimer(Time, [this, OnComplete](const float CurrentTime, const float ExecuteTime) {
		OnComplete->ExecuteIfBound(this);
	}, [this, OnUpdate, Time, OriginalLocation, OriginalRotation, Transform, EasingFunc](const float CurrentTime, const float ExecuteTime) {
		const float Amount = (CurrentTime - (ExecuteTime - Time)) / Time;
		const FVector NewLocation = UKismetMathLibrary::VEase(OriginalLocation, Transform.GetLocation(), Amount, EasingFunc);
		const FRotator NewRotator = UKismetMathLibrary::REase(OriginalRotation, Transform.GetRotation().Rotator(), Amount, true, EasingFunc);
		SetActorLocation(NewLocation);
		GetController()->SetControlRotation(NewRotator);
		OnUpdate->ExecuteIfBound(this);
	});
}

void AFPlayer::LoadMeshOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if (!OtherActor->GetClass()->ImplementsInterface(ULoadableInterface::StaticClass()) || LoadedActors.Contains(OtherActor)) return;
	ILoadableInterface::Execute_Load(OtherActor);
	LoadedActors.Add(OtherActor);
	if (ILoadableInterface::Execute_CanLoadTick(OtherActor))
		LoadedTickingActors.Add(OtherActor);
}

void AFPlayer::LoadMeshEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	if (!OtherActor->GetClass()->ImplementsInterface(ULoadableInterface::StaticClass()) || !LoadedActors.Contains(OtherActor)) return;
	ILoadableInterface::Execute_Unload(OtherActor);
	LoadedActors.Remove(OtherActor);
	if (LoadedTickingActors.Contains(OtherActor))
		LoadedTickingActors.Remove(OtherActor);
}

void AFPlayer::AddOnDestroyedListener(AActor* Actor) {
	if (!Actor || !Actor->GetClass()->ImplementsInterface(ULoadableInterface::StaticClass())) return;
	Actor->OnDestroyed.AddUniqueDynamic(this, &AFPlayer::OnActorRemoved);
}

void AFPlayer::OnActorSpawned(AActor* Actor) {
	AddOnDestroyedListener(Actor);
}

void AFPlayer::OnActorRemoved(AActor* Actor) {
	if (LoadedActors.Contains(Actor))
		LoadedActors.Remove(Actor);
}

UInterpolatePlayerAsyncAction* UInterpolatePlayerAsyncAction::InterpolatePlayerTo(AFPlayer* Player, const FTransform Transform, const float Time, const TEnumAsByte<EEasingFunc::Type> EasingFunc) {
	if (!Player) {
		LOG_ERROR("Interpolate Player must have a valid player");
		return nullptr;
	}
	UInterpolatePlayerAsyncAction* Node = NewObject<UInterpolatePlayerAsyncAction>();
	Node->Player = Player;
	Node->Time = Time;
	Node->Transform = Transform;
	Node->EasingFunc = EasingFunc;
	return Node;
}

void UInterpolatePlayerAsyncAction::Activate() {
	Player->InterpolatePlayerTo(Transform, Time, EasingFunc, [this](AFPlayer* InPlayer) {
		Update.Broadcast(InPlayer);
	}, [this](AFPlayer* InPlayer) {
		Complete.Broadcast(InPlayer);
	});
}
