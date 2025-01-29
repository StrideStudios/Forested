#include "ForestedGameMode.h"

#include "EngineUtils.h"
#include "FPlayer.h"
#include "LevelDefaults.h"
#include "ObjectSaveGame.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerInput.h"
#include "Kismet/GameplayStatics.h"

AForestedGameMode::AForestedGameMode() {
	PlayerControllerClass = AForestedPlayerController::StaticClass();

	ConstructorHelpers::FClassFinder<AFPlayer> PlayerClass(TEXT("/Game/Blueprints/Player/BP_FPlayer"));
	if (PlayerClass.Succeeded()) {
		DefaultPawnClass = PlayerClass.Class;
	}
}


void AForestedGameMode::StartPlay() {
	ULevelDefaults::StartPlay(GetWorld(), this);
	LOG("Initializing Save Game");
	InitSaveGame();
	check(ObjectSaveGame);
	InitObjects();
	switch (HasSaveData()) {
	case true:
		LoadGame();
		break;
	case false:
		LoadDefaults();
		break;
	}
	LOG("Loading Complete, Starting Game");
	Super::StartPlay();
	bGameInitialized = true;
}

void AForestedGameMode::InitSaveGame() {
	constexpr int32 Id = 0; //TODO PLAYER->GetPlatformUserId().GetInternalId()
	if (UGameplayStatics::DoesSaveGameExist(SlotName, Id)) {
		ObjectSaveGame = Cast<UObjectSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, Id));
		return;
	}
	ObjectSaveGame = Cast<UObjectSaveGame>(UGameplayStatics::CreateSaveGameObject(UObjectSaveGame::StaticClass()));
}

void AForestedGameMode::InitObjects() const {
	LOG("Initializing Objects");
	for (TObjectIterator<UObject> ObjectItr; ObjectItr; ++ObjectItr) {
		UObject* Object = *ObjectItr;
		if (!Object || !Object->GetClass()->ImplementsInterface(UInitInterface::StaticClass()))
			continue;
		//if they aren't part of this world they are invalid
		if (Object->GetWorld() != GetWorld())
			continue;
		IInitInterface::Execute_Init(Object, GetWorld());
	}
}

void AForestedGameMode::LoadGame() const {
	LOG("Loading Game From Save");
	//delete all default actors
	for (FActorIterator It(GetWorld()); It; ++It) {
		AActor* Actor = *It;
		if (Actor->GetClass()->ImplementsInterface(USaveActorInterface::StaticClass())) {
			Actor->Destroy();
		}
	}
	//load actors and tell them to load
	ObjectSaveGame->LoadActors(GetWorld(), [&](AActor* Actor) {
		ISaveActorInterface::Execute_OnGameLoad(Actor, GetWorld(), ObjectSaveGame);
	});
	//load objects and tell them to load
	for (TObjectIterator<UObject> ObjectItr; ObjectItr; ++ObjectItr) {
		UObject* Object = *ObjectItr;
		if (!Object || !Object->GetClass()->ImplementsInterface(USaveObjectInterface::StaticClass()))
			continue;
		//if they aren't part of this world they are invalid
		if (Object->GetWorld() != GetWorld())
			continue;
		ObjectSaveGame->LoadObject(Object, ISaveObjectInterface::Execute_GetTypeName(Object));
		ISaveObjectInterface::Execute_OnGameLoad(Object, GetWorld(), ObjectSaveGame);
	}
	
	//reset data
	ObjectSaveGame->Reset();
}

void AForestedGameMode::LoadDefaults() const {
	LOG("No Save Game - Loading Game Defaults");
	for (TObjectIterator<UObject> ObjectItr; ObjectItr; ++ObjectItr) {
		UObject* Object = *ObjectItr;
		if (!Object || !Object->GetClass()->ImplementsInterface(USaveInterface::StaticClass()))
			continue;
		//if they aren't part of this world they are invalid
		if (Object->GetWorld() != GetWorld())
			continue;
		ISaveInterface::Execute_LoadDefaults(Object, GetWorld());
	}
}

void AForestedGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);
	ULevelDefaults::EndPlay();
	
	//Forested Game Mode is persistent so variables need to be reset
	ObjectSaveGame = nullptr;
	bGameInitialized = false;
	bIsSaving = false;
}

bool AForestedGameMode::HasSaveData() const {
	return ObjectSaveGame->HasData();
}

void AForestedGameMode::SaveGame() {
	LOG("Save Start");
	if (ObjectSaveGame && !bIsSaving) {
		bIsSaving = true;
		//Save Actors
		for (FActorIterator It(GetWorld()); It; ++It) {
			AActor* Actor = *It;
			if (!Actor->IsChildActor() && Actor->GetClass()->ImplementsInterface(USaveActorInterface::StaticClass())) {
				ISaveActorInterface::Execute_OnGameSave(Actor, GetWorld(), ObjectSaveGame);
				ObjectSaveGame->SaveActor(Actor);
			}
		}
		//Save Objects
		for (TObjectIterator<UObject> ObjectItr; ObjectItr; ++ObjectItr) {
			UObject* Object = *ObjectItr;
			if (!Object || !Object->GetClass()->ImplementsInterface(USaveObjectInterface::StaticClass()))
				continue;
			//if they aren't part of this world they are invalid
			if (Object->GetWorld() != GetWorld())
				continue;
			ISaveObjectInterface::Execute_OnGameSave(Object, GetWorld(), ObjectSaveGame);
			ObjectSaveGame->SaveObject(Object, ISaveObjectInterface::Execute_GetTypeName(Object));
		}
		FAsyncSaveGameToSlotDelegate SavedDelegate;
		SavedDelegate.BindUObject(this, &AForestedGameMode::SaveComplete);
		UGameplayStatics::AsyncSaveGameToSlot(ObjectSaveGame, SlotName, 0, SavedDelegate);//TODO PLAYER->GetPlatformUserId().GetInternalId()
	}
}

void AForestedGameMode::SaveComplete(const FString& SaveSlotName, const int32 UserIndex, bool Success) {
	LOG("Save Complete");
	bIsSaving = false;
	ObjectSaveGame->EmptyData();
}

void AForestedGameMode::EraseGameData() const {
	LOG("Game Data Erased");
	if (ObjectSaveGame) {
		ObjectSaveGame->EmptyData();
		UGameplayStatics::SaveGameToSlot(ObjectSaveGame, SlotName, 0); //TODO PLAYER->GetPlatformUserId().GetInternalId()
	}
}

AForestedPlayerController::AForestedPlayerController() {
	bShowMouseCursor = false;
	bEnableMouseOverEvents = true;
}

/*
IE_Repeat works improperly with keyboard inputs
*/
/*
bool AForestedPlayerController::InputKey(const FInputKeyParams& Params) {
	FInputKeyParams NewParams = Params;
	if (NewParams.Event == IE_Repeat) {
		NewParams.Event = IE_Pressed;
	}
	return Super::InputKey(NewParams);
}
*/
