#include "InventoryEntry.h"

FInventoryEntry::FInventoryEntry()
	:ItemCount(0), MaxItemCount(0)
{
}

FInventoryEntry::FInventoryEntry(uint8 ItemCount, uint8 MaxItemCount)
	:ItemCount(ItemCount), MaxItemCount(MaxItemCount)
{
}
