#include "LevelDefaults.h"
#include "FPlayer.h"
#include "Sky.h"
#include "Forested/ForestedGameMode.h"
#include "Kismet/GameplayStatics.h"

AForestedGameMode* ULevelDefaults::ForestedGameMode = nullptr;
AFPlayer* ULevelDefaults::Player = nullptr;
ASky* ULevelDefaults::Sky = nullptr;

void ULevelDefaults::StartPlay(const UWorld* World, AForestedGameMode* InForestedGameMode) {
	ForestedGameMode = InForestedGameMode;
	if (AFPlayer* InPlayer = CastChecked<AFPlayer>(UGameplayStatics::GetPlayerCharacter(World, 0)))
		Player = InPlayer;
	if (ASky* InSky = CastChecked<ASky>(UGameplayStatics::GetActorOfClass(World, ASky::StaticClass())))
		Sky = InSky;
}

void ULevelDefaults::EndPlay() {
	//since these are static variables they need to be reset once the game ends
	ForestedGameMode = nullptr;
	Player = nullptr;
	Sky = nullptr;
}

AForestedGameMode* ULevelDefaults::GetForestedGameMode() {
	//Game Modes don't exist outside of gameplay
	return ForestedGameMode;
}

AFPlayer* ULevelDefaults::GetPlayer(UObject* WorldContextObject) {
	return Player ? Player : GetActor<AFPlayer>(WorldContextObject);
}

ASky* ULevelDefaults::GetSky(UObject* WorldContextObject) {
	return Sky ? Sky : GetActor<ASky>(WorldContextObject);
}