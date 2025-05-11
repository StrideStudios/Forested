#include "LevelDefaults.h"
#include "Player/FPlayer.h"
#include "Sky.h"
#include "Forested/ForestedGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Forested/ForestedMinimal.h"

ASky* ULevelDefaults::Sky = nullptr;

AForestedGameMode* ULevelDefaults::ForestedGameMode = nullptr;
AFPlayer* ULevelDefaults::Player = nullptr;

template <class UserClass>
UserClass* GetActor(const UObject* WorldContextObject) {
	if (!WorldContextObject) return nullptr;
	return Cast<UserClass>(UGameplayStatics::GetActorOfClass(WorldContextObject, UserClass::StaticClass()));
}

void ULevelDefaults::StartPlay(const UWorld* World, AForestedGameMode* InForestedGameMode) {
	Sky = GetActor<ASky>(World);
	
	ForestedGameMode = InForestedGameMode;
	Player = Cast<AFPlayer>(UGameplayStatics::GetPlayerCharacter(World, 0));
}

void ULevelDefaults::EndPlay() {
	//since these are static variables they need to be reset once the game ends
	Sky = nullptr;
	
	ForestedGameMode = nullptr;
	Player = nullptr;
}

ASky* ULevelDefaults::GetSky(const UObject* WorldContextObject) {
	return Sky ? Sky : GetActor<ASky>(WorldContextObject);
}

AForestedGameMode* ULevelDefaults::GetForestedGameMode() {
	//Game Modes don't exist outside of gameplay
	return ForestedGameMode;
}

AFPlayer* ULevelDefaults::GetPlayer() {
	return Player;
}