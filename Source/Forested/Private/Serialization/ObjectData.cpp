#include "Serialization/ObjectData.h"

FObjectData::FObjectData(const UObject* Object) {
	if (!IsValid(Object)) {
		LOG_ERROR("Saved Object Cannot Be Null");
		return;
	}
	FMemoryWriter Writer(Data, true);
	FForestedSaveGameArchive Archive(Writer);
	Object->SerializeScriptProperties(Archive);
	ObjectClass = Object->GetClass();
}

FObjectData::FObjectData(const AActor* Actor) {
	if (!IsValid(Actor)) {
		LOG_ERROR("Saved Actor Cannot Be Null");
		return;
	}
	TArray<uint8> SerializedData;
	FMemoryWriter Writer(SerializedData, true);
	FForestedSaveGameArchive Archive(Writer);
	Actor->SerializeScriptProperties(Archive);
	ObjectClass = Actor->GetClass();
	Transform = Actor->GetActorTransform();
	Data = SerializedData;
}

void FObjectData::LoadObject(UObject* Object) const {
	if (!Object) {
		LOG_ERROR("Loaded Object Was Not Found");
		return;
	}
	if (Data.Num() <= 0) {
		LOG_ERROR("Loaded Data in Object %s Not Present", *Object->GetName());
		return;
	}
	FMemoryReader Reader(Data, true);
	FForestedSaveGameArchive Archive(Reader);
	Object->SerializeScriptProperties(Archive);
}
