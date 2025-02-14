#pragma once

#include "Forested/ForestedMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Tree/TreeSubsystem.h"
#include "LevelDefaults.generated.h"

//guaranteed macros (will be active after Begin Play)
#define FORESTED_GAME_MODE ULevelDefaults::GetForestedGameMode()
#define PLAYER ULevelDefaults::GetPlayer()
#define PLAYER_INVENTORY ULevelDefaults::GetPlayer()->PlayerInventory
#define SKY ULevelDefaults::GetSky()

//non-guaranteed macros (should use if to check validity)
#define UNDERWATER_SUBSYSTEM GetWorld()->GetSubsystem<UUnderwaterSubsystem>()
#define TREE_SUBSYSTEM GetWorld()->GetSubsystem<UTreeSubsystem>()
#define TREE_INSTANCE_MANAGER TREE_SUBSYSTEM->GetTreeInstanceManager()

class AForestedGameMode;
class ASky;
class AFPlayer;
class ATreeInstanceManager;
/**
 * A Function Library to get actors which are always present in the level
 */
UCLASS()
class FORESTED_API ULevelDefaults : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	static void StartPlay(const UWorld* World, AForestedGameMode* InForestedGameMode);

	static void EndPlay();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext = "WorldContextObject"), Category = "Level Default Library|Game Mode")
	static AForestedGameMode* GetForestedGameMode();

	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext = "WorldContextObject"), Category = "Level Default Library|Player")
	static AFPlayer* GetPlayer(UObject* WorldContextObject = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext = "WorldContextObject"), Category = "Level Default Library|Sky")
	static ASky* GetSky(UObject* WorldContextObject = nullptr);

private:

	template <class UserClass>
	static UserClass* GetActor(UObject* WorldContextObject) {
		if (!WorldContextObject) return nullptr;
		LOG_WARNING("Needed to get actor from world in level defaults for actor class %s", *UserClass::StaticClass()->GetName());
		return Cast<UserClass>(UGameplayStatics::GetActorOfClass(WorldContextObject, UserClass::StaticClass()));
	}

	static AForestedGameMode* ForestedGameMode;
	
	static AFPlayer* Player;

	static ASky* Sky;
};
