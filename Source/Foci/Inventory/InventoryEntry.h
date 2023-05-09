#pragma once

#include "CoreMinimal.h"
#include "InventoryEntry.generated.h"

USTRUCT(Blueprintable)
struct FOCI_API FInventoryEntry
{
	GENERATED_BODY()

	FInventoryEntry();

	FInventoryEntry(uint8 ItemCount, uint8 MaxItemCount);

	// UPROPERTY(BlueprintReadWrite, EditAnywhere)
	// FName EntryName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	uint8 ItemCount;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	uint8 MaxItemCount;
};
