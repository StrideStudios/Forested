#pragma once

#include "Forested/ForestedMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ForestedGameMode.generated.h"

class UObjectSaveGame;

UCLASS()
class FORESTED_API AForestedGameMode : public AGameModeBase {
	GENERATED_BODY()

public:

	AForestedGameMode();

	virtual void StartPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Forested")
	FORCEINLINE bool IsGameStarted() const { return bIsGameStarted; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Forested")
	FORCEINLINE bool IsGameInitialized() const { return bGameInitialized; }
	
	UFUNCTION(BlueprintCallable, Category = "Forested")
	virtual void StartGame();

	UFUNCTION(BlueprintCallable, Category = "Forested")
	virtual void ResumeGame() const;

	UFUNCTION(BlueprintCallable, Category = "Forested")
	virtual void PauseGame() const;

	UFUNCTION(BlueprintCallable, Category = "Forested")
	virtual void QuitGame() const;
	
	void InitSaveGame();

	void InitObjects() const;

	void LoadGame() const;

	void LoadDefaults() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Loading")
	bool HasSaveData() const;
	
	UFUNCTION(BlueprintCallable)
	void SaveGame();

	UFUNCTION()
	void SaveComplete(const FString& SaveSlotName, int32 UserIndex, bool Success);

	UFUNCTION(BlueprintCallable)
	void EraseGameData() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Serialization")
	FString SlotName = "ForestedSlot0";

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Serialization")
	UObjectSaveGame* ObjectSaveGame = nullptr;

private:

	bool bGameInitialized = false;

	bool bIsGameStarted = false;

	bool bIsSaving = false;
};


UINTERFACE(MinimalAPI, Blueprintable)
class UInitInterface : public UInterface {
	GENERATED_BODY()
};

/*
Used for initializing objects before begin play is called and before game is loaded
*/
class FORESTED_API IInitInterface {
	GENERATED_BODY()

public:

	/*
	Initialization function for objects, this occurs before begin play on any component or actor
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, meta= (WorldContext = "WorldContextObject"))
	void Init(UObject* WorldContextObject);
	virtual void Init_Implementation(UObject* WorldContextObject) {}
	
};

UCLASS()
class FORESTED_API AForestedPlayerController : public APlayerController {
	GENERATED_BODY()

public:

	AForestedPlayerController();

	//virtual bool InputKey(const FInputKeyParams& Params) override;

	//virtual bool InputKey(FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad) override;

};