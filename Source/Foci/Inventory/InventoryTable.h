#pragma once

#include "CoreMinimal.h"

#include "InventoryEntry.h"

#include "InventoryTable.generated.h"

UCLASS()
class FOCI_API UInventoryTable : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = true))
	TMap<FName, FInventoryEntry> InventoryMap;

public:
	/// Returns false if we can't give the item -- either this will give more than the max capacity, or the item name doesn't exist
	bool GiveItem(FName ItemName, uint8 ItemCount);

	/// Returns true if we have enough items in the inventory, will not consume items on false return
	bool ConsumeItem(FName ItemName, uint8 ItemCount);

	uint8 GetItemCount(FName ItemName) const;

	bool IsFull(FName ItemName) const;

	void UnlockItem(FName ItemName, uint8 MaxItemCount);

	bool SetInventoryCapacity(FName ItemName, uint8 NewMax, bool bForceInventoryShrink = false);
};
