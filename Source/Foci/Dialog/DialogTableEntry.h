#pragma once

#include "CoreMinimal.h"

#include "Engine/DataTable.h" 
#include "DialogResponse.h"

#include "DialogTableEntry.generated.h"

USTRUCT(BlueprintType)
struct FOCI_API FDialogTableEntry : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FText PreviousDialogInstance;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	uint8 OptionChosen = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FDialogResponse DialogResponse;
};
