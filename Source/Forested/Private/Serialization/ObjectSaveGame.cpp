#include "Serialization/ObjectSaveGame.h"

void UObjectSaveGame::SaveActor(AActor* Actor) {
	if (Actor->IsChildActor() || !Actor->GetClass()->ImplementsInterface(USaveInterface::StaticClass()))
		return;
	FObjectData Data = FObjectData(Actor);
	Data.Transform = Actor->GetActorTransform();
	ActorData.Push(Data);
}

AActor* UObjectSaveGame::LoadActors_Internal(UWorld* World, const FObjectData& Data) {
	return Data.LoadActor<AActor>(World);
}

void UObjectSaveGame::SaveObject(UObject* Object, const FString& TypeName) {
	ObjectData.Add(TypeName, FObjectData(Object));
}

void UObjectSaveGame::LoadObject(UObject* Object, const FString& TypeName) {
	if (!ObjectData.Contains(TypeName)) return;
	FObjectData Data;
	ObjectData.RemoveAndCopyValue(TypeName, Data);
	Data.LoadObject(Object);
}
