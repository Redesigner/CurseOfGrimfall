#include "InventoryTable.h"

#include "Foci/Foci.h"

bool UInventoryTable::GiveItem(FName ItemName, uint8 ItemCount)
{
	FInventoryEntry* Item = InventoryMap.Find(ItemName);
	if (!Item)
	{
		UE_LOG(LogInventory, Display, TEXT("Unable to give item to inventory at '%s': item does not exist in the inventory map."), *GetPathNameSafe(this))
		return false;
	}
	if (Item->ItemCount + ItemCount > Item->MaxItemCount)
	{
		Item->ItemCount = Item->MaxItemCount;
		OnInventoryItemCountChanged.Broadcast(ItemName, Item->ItemCount);
		return false;
	}
	Item->ItemCount += ItemCount;
	OnInventoryItemCountChanged.Broadcast(ItemName, Item->ItemCount);
	return true;
}

bool UInventoryTable::ConsumeItem(FName ItemName, uint8 ItemCount)
{
	FInventoryEntry* Item = InventoryMap.Find(ItemName);
	if (!Item)
	{
		UE_LOG(LogInventory, Display, TEXT("Unable to consume item from inventory at '%s': item does not exist in the inventory map."), *GetPathNameSafe(this))
		return false;
	}
	if (Item->ItemCount < ItemCount)
	{
		return false;
	}
	Item->ItemCount -= ItemCount;
	OnInventoryItemCountChanged.Broadcast(ItemName, Item->ItemCount);
	return true;
}

uint8 UInventoryTable::GetItemCount(FName ItemName) const
{
	const FInventoryEntry* Item = InventoryMap.Find(ItemName);
	if (!Item)
	{
		UE_LOG(LogInventory, Display, TEXT("Unable to find inventory item '%s' at '%s': item does not exist in the inventory map."), *ItemName.ToString(), *GetPathNameSafe(this))
		return 0;
	}
	return Item->ItemCount;
}

bool UInventoryTable::IsFull(FName ItemName) const
{
	return false;
}

void UInventoryTable::UnlockItem(FName ItemName, uint8 MaxItemCount)
{
	const FInventoryEntry* Item = InventoryMap.Find(ItemName);
	if (Item)
	{
		UE_LOG(LogInventory, Display, TEXT("Unable to unlock inventory item at '%s': item '%s' already unlocked."), *GetPathNameSafe(this), *ItemName.ToString())
		return;
	}
	InventoryMap.Add(ItemName, FInventoryEntry(0, MaxItemCount));
}

bool UInventoryTable::SetInventoryCapacity(FName ItemName, uint8 NewMax, bool bForceInventoryShrink)
{
	FInventoryEntry* Item = InventoryMap.Find(ItemName);
	if (!Item)
	{
		UE_LOG(LogInventory, Display, TEXT("Unable to find inventory item '%s' at '%s': item does not exist in the inventory map."), *ItemName.ToString(), *GetPathNameSafe(this))
		return false;
	}
	if (bForceInventoryShrink)
	{
		Item->MaxItemCount = NewMax;
		return true;
	}
	if (NewMax > Item->MaxItemCount)
	{
		return false;
	}
	Item->MaxItemCount = NewMax;
	return true;
}
