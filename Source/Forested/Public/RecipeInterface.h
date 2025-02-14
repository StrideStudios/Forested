#pragma once

#include "Forested/ForestedMinimal.h"
#include "UObject/Interface.h"
#include "Item/Item.h"
#include "RecipeInterface.generated.h"

/*
 * reasons to include this:
 * expandability at the cost of flexibility
 * 
 */

UCLASS(BlueprintType, Blueprintable)
class FORESTED_API URecipe : public UObject {
    GENERATED_BODY()

public:
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Recipe")
    TArray<FItemHeap> GetIngredients();
    virtual TArray<FItemHeap> GetIngredients_Implementation() {
        return {};
    }
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Recipe")
    FItemHeap GetResult();
    virtual FItemHeap GetResult_Implementation() {
        return {};
    }
};

UINTERFACE(MinimalAPI, Blueprintable)
class UCraftingInterface : public UInterface {
    GENERATED_BODY()
};

class FORESTED_API ICraftingInterface {
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Crafting")
    URecipe* GetRecipe(FItemHeap Result);
    virtual URecipe* GetRecipe_Implementation(FItemHeap Result) {
        return nullptr;
    }
};
