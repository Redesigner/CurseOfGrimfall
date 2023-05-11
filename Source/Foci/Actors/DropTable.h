#pragma once

#include "CoreMinimal.h"

#include "DropTableEntry.h"

#include "DropTable.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class FOCI_API UDropTable : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	TArray<FDropTableEntry> DropTable;

public:
	TArray<TSubclassOf<APickup>> GetDroppedItems() const;

	UFUNCTION(BlueprintCallable)
	void SpawnDrops(FVector SpawnLocation, FRotator SpawnRotation, float BurstSpeed = 0.0f);
};
