#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LevelDefaults.generated.h"

/*
 * guaranteed macros (will be active after game mode Start Play)
 */

#define FORESTED_GAME_MODE ULevelDefaults::GetForestedGameMode()
#define PLAYER ULevelDefaults::GetPlayer()
#define PLAYER_INVENTORY ULevelDefaults::GetPlayer()->PlayerInventory
#define SKY ULevelDefaults::GetSky()

/**
 * A Function Library to get actors which are always present in the level
 */
UCLASS()
class FORESTED_API ULevelDefaults : public UBlueprintFunctionLibrary {
	GENERATED_BODY()

	static class AForestedGameMode* ForestedGameMode;
	
	static class AFPlayer* Player;

	static class ASky* Sky;

public:
	
	static void StartPlay(const UWorld* World, AForestedGameMode* InForestedGameMode);

	static void EndPlay();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext = "WorldContextObject"), Category = "Level Default Library|Game Mode")
	static AForestedGameMode* GetForestedGameMode();

	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext = "WorldContextObject"), Category = "Level Default Library|Player")
	static AFPlayer* GetPlayer(UObject* WorldContextObject = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext = "WorldContextObject"), Category = "Level Default Library|Sky")
	static ASky* GetSky(UObject* WorldContextObject = nullptr);

};
