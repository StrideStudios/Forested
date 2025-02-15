#include "LevelDefaults.h"
#include "FPlayer.h"
#include "Sky.h"
#include "Forested/ForestedGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Forested/ForestedMinimal.h"

AForestedGameMode* ULevelDefaults::ForestedGameMode = nullptr;
AFPlayer* ULevelDefaults::Player = nullptr;
ASky* ULevelDefaults::Sky = nullptr;

template <class UserClass>
UserClass* GetActor(UObject* WorldContextObject) {
	if (!WorldContextObject) return nullptr;
	LOG_WARNING("Attempted to get actor from world in level defaults for actor class %s", *UserClass::StaticClass()->GetName());
	return Cast<UserClass>(UGameplayStatics::GetActorOfClass(WorldContextObject, UserClass::StaticClass()));
}

void ULevelDefaults::StartPlay(const UWorld* World, AForestedGameMode* InForestedGameMode) {
	ForestedGameMode = InForestedGameMode;
	Player = Cast<AFPlayer>(UGameplayStatics::GetPlayerCharacter(World, 0));
	Sky = Cast<ASky>(UGameplayStatics::GetActorOfClass(World, ASky::StaticClass()));
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