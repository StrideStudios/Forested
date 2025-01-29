#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "UObject/Interface.h"
#include "ObjectData.h"
#include "Forested/Forested.h"
#include "ObjectSaveGame.generated.h"

/* https://forums.unrealengine.com/t/how-to-can-you-store-an-object-into-a-savegame/605261/4
 * this helped a lot, thanks
 * next few comments also list problems w/ blueprints, so good to keep an eye on
 * "If you change the variables of the item you saved, applying the serialization will fail."
 * essentially, do not change variables unless you intend to delete player data
 * "Modifying IDs of old properties will not work."
 * good to add conversions or such
 * true for probably any saving system, keep an eye out for it
 */

class AFPlayer;

UCLASS(BlueprintType)
class FORESTED_API UObjectSaveGame : public USaveGame {
	GENERATED_BODY()

public:

	void SaveActor(AActor* Actor);

	void LoadActors(UWorld* World, const TFunction<void(AActor*)>& Function) {
		while (ActorData.Num() > 0) {
			const FObjectData Data = ActorData.Pop();
			if (!Data) continue;
			if (AActor* Actor = LoadActors_Internal(World, Data)) {
				Function(Actor);
			}
		}
	}

	void SaveObject(UObject* Object, const FString& TypeName);

	void LoadObject(UObject* Object, const FString& TypeName);

	void Reset() {
		for (auto& Data : ObjectData) {
			LOG_WARNING("Object with Type Name %s was not loaded", *Data.Key);
		}
		for (const auto& Data : ActorData) {
			if (!Data.ObjectClass) continue;
			LOG_WARNING("Actor with Name %s was not loaded", *Data.ObjectClass->GetName());
		}
		EmptyData();
	}
	
	FORCEINLINE bool HasData() const {
		return ObjectData.Num() || ActorData.Num();
	}

	FORCEINLINE void EmptyData() {
		ObjectData.Empty();
		ActorData.Empty();
	}

	UPROPERTY(SaveGame)
	TMap<FString, FObjectData> ObjectData;

	UPROPERTY(SaveGame)
	TArray<FObjectData> ActorData;

private:
	static AActor* LoadActors_Internal(UWorld* World, const FObjectData& Data);
};

//do with this


UINTERFACE(MinimalAPI, Blueprintable)
class USaveInterface : public UInterface {
	GENERATED_BODY()
};

/*
Used for saving the game.  Any UPROPERTY with the SaveGame flag is AUTOMATICALLY saved AND loaded with this interface (so long as it is an actor)
Saving is done before writing to disk while loading is done after (duh)
Do not confuse this with ILoadableInterface, which is used for loading an object when a player is in range, I am just too dumb to find another name for it
*/
class FORESTED_API ISaveInterface {
	GENERATED_BODY()

public:

	/*
	can be used to set variables before being saved
	useful for saving child actor data, see TreeActor.cpp for an example
	remember overriding this in blueprints will remove the c++ implementation unless you call to the parent function
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, meta= (WorldContext = "WorldContextObject"))
	void OnGameSave(UObject* WorldContextObject, UObjectSaveGame* SaveGame);
	virtual void OnGameSave_Implementation(UObject* WorldContextObject, UObjectSaveGame* SaveGame) {}

	/*
	Good for enabling certain behaviors with variables
	useful for loading child actor data, see TreeActor.cpp for an example
	remember overriding this in blueprints will remove the c++ implementation unless you call to the parent function
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, meta= (WorldContext = "WorldContextObject"))
	void OnGameLoad(UObject* WorldContextObject, UObjectSaveGame* SaveGame);
	virtual void OnGameLoad_Implementation(UObject* WorldContextObject, UObjectSaveGame* SaveGame) {}

	/*
	Good for setting default values for saved data, will only be called upon a new game
	remember overriding this in blueprints will remove the c++ implementation unless you call to the parent function
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, meta= (WorldContext = "WorldContextObject"))
	void LoadDefaults(UObject* WorldContextObject);
	virtual void LoadDefaults_Implementation(UObject* WorldContextObject) {}
	
};

UINTERFACE(MinimalAPI, Blueprintable)
class USaveObjectInterface : public USaveInterface {
	GENERATED_BODY()
};

class FORESTED_API ISaveObjectInterface : public ISaveInterface {
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FString GetTypeName() const;
	virtual FString GetTypeName_Implementation() const {
		return FString();
	}
	
};

UINTERFACE(MinimalAPI, Blueprintable)
class USaveActorInterface : public USaveInterface {
	GENERATED_BODY()
};

class FORESTED_API ISaveActorInterface : public ISaveInterface {
	GENERATED_BODY()
	//could add custom stuff
};