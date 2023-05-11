#include "DropTable.h"

#include "GameFramework/MovementComponent.h" 

TArray<TSubclassOf<APickup>> UDropTable::GetDroppedItems() const
{
	TArray<TSubclassOf<APickup>> Pickups;

	for (FDropTableEntry DropTableEntry : DropTable)
	{
		const float Random = FMath::RandRange(0, 1);
		if (Random <= DropTableEntry.DropRate)
		{
			Pickups.Add(DropTableEntry.Item);
		}
	}
	return Pickups;
}

void UDropTable::SpawnDrops(FVector SpawnLocation, FRotator SpawnRotation, float BurstSpeed)
{
	if (!GetWorld())
	{
		return;
	}
	TArray<TSubclassOf<APickup>> Pickups = GetDroppedItems();

	for (TSubclassOf<APickup> Pickup : Pickups)
	{
		AActor* Actor = GetWorld()->SpawnActor(Pickup, &SpawnLocation, &SpawnRotation);
		float BurstAngle = FMath::RandRange(-PI, PI);
		FVector BurstVelocity = FVector(FMath::Cos(BurstAngle), FMath::Sin(BurstAngle), 0.0f) * BurstSpeed;

		APickup* PickupActor = Cast<APickup>(Actor);
		PickupActor->Velocity = (BurstVelocity);
		PickupActor->SetMoving(true);
	}
}
