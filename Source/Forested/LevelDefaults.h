#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LevelDefaults.generated.h"

/*
 * Guaranteed Macros (actors that are available in the world at all times)
 * Cached at begin play for faster getting, should never return null
 * In most situations, 'this' will suffice for getting the world, but occasionally you will need an override
 */

#define GET_SKY(Object) ULevelDefaults::GetSky(Object)
#define SKY GET_SKY(this)

/*
 * Spawned Macros (will be active after game mode Start Play)
 * These should be used with a validity check
 */

#define FORESTED_GAME_MODE ULevelDefaults::GetForestedGameMode()
#define PLAYER ULevelDefaults::GetPlayer()
#define PLAYER_INVENTORY ULevelDefaults::GetPlayer()->PlayerInventory

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

	/*
	 * Guaranteed
	 */
	
	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext = "WorldContextObject"), Category = "Level Default Library|Sky")
	static ASky* GetSky(const UObject* WorldContextObject = nullptr);

	/*
	 * Spawned
	 */
	
	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext = "WorldContextObject"), Category = "Level Default Library|Game Mode")
	static AForestedGameMode* GetForestedGameMode();

	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext = "WorldContextObject"), Category = "Level Default Library|Player")
	static AFPlayer* GetPlayer();

};
