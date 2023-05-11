#pragma once

#include "CoreMinimal.h"

#include "Foci/Inventory/Pickup.h"

#include "DropTableEntry.generated.h"

USTRUCT(Blueprintable)
struct FOCI_API FDropTableEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0f, ClampMax = 1.0f))
	float DropRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<APickup> Item;
};
