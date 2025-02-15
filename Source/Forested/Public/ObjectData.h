#pragma once

#include "Forested/ForestedMinimal.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "ObjectData.generated.h"

struct FForestedSaveGameArchive final : FObjectAndNameAsStringProxyArchive {
    explicit FForestedSaveGameArchive(FArchive& InInnerArchive): 
        FObjectAndNameAsStringProxyArchive(InInnerArchive, true) {
        ArIsSaveGame = true;
        ArNoDelta = true;
    }
};

USTRUCT(BlueprintType)
struct FObjectData {
    GENERATED_BODY()

    FObjectData():
    ObjectClass(nullptr),
    Data({}),
    Transform(FTransform::Identity) {
    }

    explicit FObjectData(const UObject* Object);

    explicit FObjectData(const AActor* Actor);

    template <class UserClass>
    UserClass* LoadNewObject(UObject* Outer) const {
        if (!ObjectClass) return nullptr;
        UserClass* Object = NewObject<UserClass>(Outer, ObjectClass);
        LoadObject(Object);
        return Object;
    }

    void LoadObject(UObject* Object) const;

    template <class UserClass>
    UserClass* LoadActor(UWorld* World) const {
        if (!ObjectClass) return nullptr;
        UserClass* Actor = World->SpawnActorDeferred<UserClass>(ObjectClass, Transform);
        LoadObject(Actor);
        Actor->FinishSpawning(Transform);
        return Actor;
    }

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame)
    TSubclassOf<UObject> ObjectClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame)
    TArray<uint8> Data;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame)
    FTransform Transform;

    FORCEINLINE operator bool() const {
        return IsValid(ObjectClass);
    }

    FORCEINLINE bool operator==(const FObjectData& ObjectData) const {
        return ObjectClass == ObjectData.ObjectClass && Data == ObjectData.Data && Transform.Equals(ObjectData.Transform);
    }
	
    FORCEINLINE operator UClass*() const {
        return ObjectClass.Get();
    }
	
    FORCEINLINE operator TSubclassOf<UObject>() const {
        return ObjectClass;
    }

    FORCEINLINE TSubclassOf<UObject> operator*() const {
        return ObjectClass;
    }

    FORCEINLINE TSubclassOf<UObject> operator->() const {
        return ObjectClass;
    }
    
};