#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LoadableInterface.generated.h"

class AFPlayer;

/*
Used for loading and unloading an object when a player enters within a certain range (only works with actors)
TODO: Actors are automatically unloaded upon begin play and will be loaded when within range
Ex: Trees use this to enable their tick function to allow proper collision within range of the player
This should not be used to cull the object or disable physics as both are handled by the level's cull volume
If you wish to set an override for this, do so within the components or a more localized cull volume.
*/
UINTERFACE(MinimalAPI, Blueprintable)
class ULoadableInterface : public UInterface {
	GENERATED_BODY()
};

class FORESTED_API ILoadableInterface {
	GENERATED_BODY()

public:

	/*
	Whether or not this actor will tick when loaded, if false the logic of ticking this actor with loadTick can't be ran
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool CanLoadTick();
	virtual bool CanLoadTick_Implementation() { return false; }
	
	/*
	While this actor is in range this runs every tick, useful for defining logic only needed within the player's range
	remember overriding this in blueprints will remove the c++ implementation unless you call to the parent function
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void LoadTick();
	virtual void LoadTick_Implementation() {}
	
	/*
	The player has gotten close enough for this object to be loaded
	remember overriding this in blueprints will remove the c++ implementation unless you call to the parent function
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Load();
	virtual void Load_Implementation() {}

	/*
	The player has gone far enough away for this object to be unloaded
	remember overriding this in blueprints will remove the c++ implementation unless you call to the parent function
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Unload();
	virtual void Unload_Implementation() {}

};
