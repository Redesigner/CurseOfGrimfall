#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Pickup.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class FOCI_API APickup : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	FName ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	uint8 ItemCount;

public:
	UFUNCTION(BlueprintImplementableEvent)
	void OnPickup(class AFociCharacter* Character);

	UFUNCTION(BlueprintCallable)
	FName GetItemName();

	UFUNCTION(BlueprintCallable)
	uint8 GetItemCount();
};
