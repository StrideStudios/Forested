#include "Item/K2Node_ConstructItem.h"
#include "K2Node_CallFunction.h"
#include "KismetCompilerMisc.h"
#include "KismetCompiler.h"
#include "Item/Item.h"
#include "Kismet2/BlueprintEditorUtils.h"

#define LOCTEXT_NAMESPACE "K2Node_ConstructItem"

UK2Node_ConstructItem::UK2Node_ConstructItem(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer) {
	NodeTooltip = LOCTEXT("NodeTooltip", "Attempts to spawn a new Item");
}

FText UK2Node_ConstructItem::GetNodeTitle(const ENodeTitleType::Type TitleType) const {
	FText NodeTitle = NSLOCTEXT("K2Node", "ConstructItem_BaseTitle", "Construct Item from Class");
	if (TitleType != ENodeTitleType::MenuTitle) {
		if (const UEdGraphPin* ClassPin = GetClassPin()) {
			if (ClassPin->LinkedTo.Num() > 0) {
				// Blueprint will be determined dynamically, so we don't have the name in this case
				NodeTitle = NSLOCTEXT("K2Node", "ConstructItem_Title_Unknown", "Construct Item");
			}
			else if (ClassPin->DefaultObject == nullptr) {
				NodeTitle = NSLOCTEXT("K2Node", "ConstructItem_Title_NONE", "Construct Item NONE");
			} else {
				if (CachedNodeTitle.IsOutOfDate(this)) {
					FText ClassName;
					if (const UClass* PickedClass = Cast<UClass>(ClassPin->DefaultObject)) {
						ClassName = PickedClass->GetDisplayNameText();
					}

					FFormatNamedArguments Args;
					Args.Add(TEXT("ClassName"), ClassName);

					// FText::Format() is slow, so we cache this to save on performance
					CachedNodeTitle.SetCachedText(FText::Format(NSLOCTEXT("K2Node", "ConstructItem_Title_Class", "Construct Item {ClassName}"), Args), this);
				}
				NodeTitle = CachedNodeTitle;
			} 
		} else {
			NodeTitle = NSLOCTEXT("K2Node", "ConstructItem_Title_NONE", "Construct Item NONE");
		}
	}
	return NodeTitle;
}

void UK2Node_ConstructItem::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) {
	Super::ExpandNode(CompilerContext, SourceGraph);
	
	UEdGraphPin* SpawnExecPin = GetExecPin();
	UEdGraphPin* SpawnWorldContextPin = GetWorldContextPin();
	UEdGraphPin* SpawnClassPin = GetClassPin();

	UEdGraphPin* SpawnResultPin = GetResultPin();

	// store off the class to spawn before we mutate pin connections:
	const UClass* ClassToSpawn = GetClassToSpawn();

	UClass* SpawnClass = SpawnClassPin != nullptr ? Cast<UClass>(SpawnClassPin->DefaultObject) : nullptr;
	if (!SpawnClassPin || (0 == SpawnClassPin->LinkedTo.Num() && !SpawnClass)) {
		CompilerContext.MessageLog.Error(*LOCTEXT("SpawnActorNodeMissingClass_Error", "Spawn node @@ must have a @@ specified.").ToString(), this, SpawnClassPin);
		// we break exec links so this is the only error we get, don't want the SpawnActor node being considered and giving 'unexpected node' type warnings
		BreakAllNodeLinks();
		return;
	}
	
	UK2Node_CallFunction* CallCreateNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	CallCreateNode->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UItem, ConstructItem), UItem::StaticClass());
	CallCreateNode->AllocateDefaultPins();

	UEdGraphPin* CallExecPin = CallCreateNode->GetExecPin();
	UEdGraphPin* CallWorldContextPin = CallCreateNode->FindPinChecked(TEXT("WorldContextObject"));
	UEdGraphPin* CallClassPin = CallCreateNode->FindPin(TEXT("ItemClass"));
	
	UEdGraphPin* CallResultPin = CallCreateNode->GetReturnValuePin();

	CompilerContext.MovePinLinksToIntermediate(*SpawnExecPin, *CallExecPin);

	if(SpawnClassPin->LinkedTo.Num() > 0) {
		// Copy the 'blueprint' connection from the spawn node to 'begin spawn'
		CompilerContext.MovePinLinksToIntermediate(*SpawnClassPin, *CallClassPin);
	} else {
		// Copy blueprint literal onto begin spawn call 
		CallClassPin->DefaultObject = SpawnClass;
	}

	if (SpawnWorldContextPin) {
		CompilerContext.MovePinLinksToIntermediate(*SpawnWorldContextPin, *CallWorldContextPin);
	}

	// Move result connection from spawn node to 'finish spawn'
	CallResultPin->PinType = SpawnResultPin->PinType; // Copy type so it uses the right actor subclass
	CompilerContext.MovePinLinksToIntermediate(*SpawnResultPin, *CallResultPin);

	//assign exposed values and connect then
	
	UEdGraphPin* LastThen = FKismetCompilerUtilities::GenerateAssignmentNodes(CompilerContext, SourceGraph, CallCreateNode, this, CallResultPin, ClassToSpawn);
	UEdGraphPin* SpawnNodeThen = GetThenPin();
	CompilerContext.MovePinLinksToIntermediate(*SpawnNodeThen, *LastThen);

	BreakAllNodeLinks();
}

void UK2Node_ConstructItem::EarlyValidation(FCompilerResultsLog& MessageLog) const {
	Super::EarlyValidation(MessageLog);
	const UClass* ClassToSpawn = GetClassToSpawn();
	if (!ClassToSpawn || ClassToSpawn->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists)) {
		MessageLog.Error(*FText::Format(LOCTEXT("ConstructItem_WrongClassFmt", "Cannot construct objects of type '{0}' in @@"), FText::FromString(GetPathNameSafe(ClassToSpawn))).ToString(), this);
	}
}

bool UK2Node_ConstructItem::IsCompatibleWithGraph(const UEdGraph* TargetGraph) const {
	const UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(TargetGraph);
	return Super::IsCompatibleWithGraph(TargetGraph) && (!Blueprint || (FBlueprintEditorUtils::FindUserConstructionScript(Blueprint) != TargetGraph && Blueprint->GeneratedClass->GetDefaultObject()->ImplementsGetWorld()));
}

void UK2Node_ConstructItem::GetNodeAttributes(TArray<TKeyValuePair<FString, FString>>& OutNodeAttributes) const {
	const UClass* ClassToSpawn = GetClassToSpawn();
	const FString ClassToSpawnStr = ClassToSpawn ? ClassToSpawn->GetName() : TEXT( "InvalidClass" );
	OutNodeAttributes.Add( TKeyValuePair<FString, FString>( TEXT( "Type" ), TEXT( "ConstructItem" ) ));
	OutNodeAttributes.Add( TKeyValuePair<FString, FString>( TEXT( "Class" ), GetClass()->GetName() ));
	OutNodeAttributes.Add( TKeyValuePair<FString, FString>( TEXT( "Name" ), GetName() ));
	OutNodeAttributes.Add( TKeyValuePair<FString, FString>( TEXT( "ItemClass" ), ClassToSpawnStr ));	
}

UClass* UK2Node_ConstructItem::GetClassPinBaseClass() const {
	return UItem::StaticClass();
}

#undef LOCTEXT_NAMESPACE
