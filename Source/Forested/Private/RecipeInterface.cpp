#include "RecipeInterface.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Item/AxeItem.h"
/*
void ALevelManager::BeginPlay() {
	
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	AssetRegistry.ScanPathsSynchronous({ TEXT("/Game/") });
	TArray<FAssetData> ScriptAssetList;
	AssetRegistry.GetAssetsByPath(FName("/Game/"), ScriptAssetList, true);
	for (const FAssetData& Asset : ScriptAssetList) {
		const UBlueprint* BlueprintObj = Cast<UBlueprint>(Asset.GetAsset());
		if (!BlueprintObj)
			continue;
		UClass* BlueprintClass = BlueprintObj->GeneratedClass;
		if (!BlueprintClass || !BlueprintClass->IsChildOf(UC_Item::StaticClass()))
			continue;
		RegisteredItems.Emplace("",BlueprintClass);
	}
	
}
*/
/*
 * IRecipe interface (ingredients and result)
 * Workbench recipe extends UObjects implements UWorkbench
 * Two Ingredient uproperties and one output item
 * 
 */