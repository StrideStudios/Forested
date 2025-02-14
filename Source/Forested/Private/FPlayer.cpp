#include "FPlayer.h"

#include "EngineUtils.h"
#include "GameFramework/PlayerController.h" 
#include "Item/PlayerInventory.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h" 
#include "GameFramework/CharacterMovementComponent.h" 
#include "Kismet/GameplayStatics.h"
#include "LevelDefaults.h"
#include "LoadableInterface.h"
#include "PlayerInputComponent.h"
#include "PlayerWidget.h"
#include "SelectableInterface.h"
#include "Sky.h"
#include "ViewmodelMeshes.h"
#include "Components/SphereComponent.h"
#include "Engine/GameViewportClient.h"
#include "Kismet/KismetMathLibrary.h"

AFPlayer::AFPlayer(const FObjectInitializer& ObjectInitializer) :
Super(ObjectInitializer.SetDefaultSubobjectClass<UPlayerMovementComponent>(CharacterMovementComponentName).SetDefaultSubobjectClass<UViewmodelSkeletalMeshComponent>(MeshComponentName)){
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	
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
	ItemMesh->SetVisibility(false);
	
	BreathingPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Breathing Point"));
	BreathingPoint->SetupAttachment(GetCapsuleComponent());
	BreathingPoint->SetRelativeLocation(Camera->GetRelativeLocation() + FVector(0.0, 0.0, 15.0));
	WaistPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Waist Point"));
	WaistPoint->SetupAttachment(GetCapsuleComponent());
	WaistPoint->SetRelativeLocation(Camera->GetRelativeLocation() - FVector(0.0, 0.0, 15.0));

	PlayerInputComponent = CreateDefaultSubobject<UPlayerInputComponent>(TEXT("Player Input Component"));
	PlayerInventory = CreateDefaultSubobject<UPlayerInventory>(TEXT("Inventory"));
}

void AFPlayer::BeginPlay() {
	Super::BeginPlay();

	//assert that the hud class is valid
	check(!PlayerHudClass.IsNull());
	
	FSerializationLibrary::LoadSync(PlayerHudClass);
	PlayerHud = UPlayerWidget::CreatePlayerWidget<UPlayerHud>(this, PlayerHudClass.Get());

	//assert the hud was created
	check(PlayerHud);
	
	for (int i = 0; i < PlayerInventory->GetCapacity(); ++i) {
		PlayerHud->UpdateSlot(i);
	}

	//add on destroy listeners for all current actors in the world
	for (TActorIterator<AActor> It(GetWorld()); It; ++It) {
		AddOnDestroyedListener(*It);
	}

	//each actor spawned will have an on destroyed listener added
	GetWorld()->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateUObject(this, &AFPlayer::OnActorSpawned));
	
	PlayerHud->SetSlotSelected(PlayerInventory->GetSelectedSlot(), true);
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

bool AFPlayer::GetPlayerAnimInstance(UPlayerAnimInstance*& OutAnimInstance) const {
	if (!GetMesh() || !GetMesh()->GetAnimInstance()) return false;
	OutAnimInstance = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	return true;
}

bool AFPlayer::IsInputAllowed() const {
	return !GetWorld()->GetGameViewport()->IgnoreInput();
}

//WORKS ON FIRST, NOT AFTER
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
	}//leaf ui doesnt show
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

//seems to teleport on first frame
void AFPlayer::InterpolatePlayerTo_Internal(const FTransform& Transform, float Time, TEnumAsByte<EEasingFunc::Type> EasingFunc, FInterpolatePlayerDelegate OnUpdate, FInterpolatePlayerDelegate OnComplete) {
	const FVector& OriginalLocation = GetActorLocation();
	const FRotator& OriginalRotation = GetControlRotation();
	SKY->AddTimer(Time, [this, OnComplete](const float CurrentTime, const float ExecuteTime) {
		OnComplete.ExecuteIfBound(this);
	}, [this, OnUpdate, Time, OriginalLocation, OriginalRotation, Transform, EasingFunc](const float CurrentTime, const float ExecuteTime) {
		const float Amount = (CurrentTime - (ExecuteTime - Time)) / Time;
		const FVector NewLocation = UKismetMathLibrary::VEase(OriginalLocation, Transform.GetLocation(), Amount, EasingFunc);
		const FRotator NewRotator = UKismetMathLibrary::REase(OriginalRotation, Transform.GetRotation().Rotator(), Amount, true, EasingFunc);
		SetActorLocation(NewLocation);
		GetController()->SetControlRotation(NewRotator);
		OnUpdate.ExecuteIfBound(this);
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

bool AFPlayer::IsInMenu() const {
	return PlayerHud->IsInMenu();
}

void AFPlayer::StartGame() {
	bIsGameStarted = true;
	PlayerHud->SetGameHud();
}

void AFPlayer::ResumeGame() const {
	PlayerHud->SetGameHud();
}

void AFPlayer::PauseGame() const {
	PlayerHud->SetMenuHud();
}

void AFPlayer::QuitGame() const {
	APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
	UKismetSystemLibrary::QuitGame(this, PlayerController, EQuitPreference::Quit, false);
}

void AFPlayer::ShowHud() const {
	PlayerHud->SetWidgetVisible();
}

void AFPlayer::HideHud() const {
	PlayerHud->SetWidgetCollapsed();
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
