#pragma once

#include "CoreMinimal.h"
#include "Pickup.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class FOCI_API APickup : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	FName ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	uint8 ItemCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	bool bIsMoving = false;

public:
	APickup(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnPickup(class AFociCharacter* Character);

	UFUNCTION(BlueprintCallable)
	FName GetItemName();

	UFUNCTION(BlueprintCallable)
	uint8 GetItemCount();

	UFUNCTION(BlueprintCallable)
	void SetMoving(bool bMoving);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Velocity;
};
