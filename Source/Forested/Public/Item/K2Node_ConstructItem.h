#pragma once

#if WITH_EDITOR

#include "CoreMinimal.h"
#include "K2Node_ConstructObjectFromClass.h"
#include "K2Node_ConstructItem.generated.h"

class UEdGraph;

UCLASS()
class FORESTED_API UK2Node_ConstructItem : public UK2Node_ConstructObjectFromClass {
	GENERATED_BODY()

	UK2Node_ConstructItem(const FObjectInitializer& ObjectInitializer);

	//~ Begin UEdGraphNode Interface.
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual void EarlyValidation(FCompilerResultsLog& MessageLog) const override;
	virtual bool IsCompatibleWithGraph(const UEdGraph* TargetGraph) const override;
	//~ End UEdGraphNode Interface.

	//~ Begin UK2Node Interface
	virtual void GetNodeAttributes( TArray<TKeyValuePair<FString, FString>>& OutNodeAttributes ) const override;
	//~ End UK2Node Interface

	//~ Begin UK2Node_ConstructObjectFromClass Interface
	virtual UClass* GetClassPinBaseClass() const override;
	//~ End UK2Node_ConstructObjectFromClass Interface

	virtual bool UseOuter() const override { return false; }
};

#endif