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
	uint8 ItemCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, meta = (AllowPrivateAccess = true))
	float DisplayHeight = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, meta = (AllowPrivateAccess = true))
	class USoundBase* PickupSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, meta = (AllowPrivateAccess = true))
	float DisplayTime = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	bool bIsMoving = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	bool bActive = true;

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void OnPickup(class AFociCharacter* Character);

public:
	APickup(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaTime) override;

	/// Returns true if player can pick this up, otherwise, returns false
	bool Pickup(class AFociCharacter* Character);

	UFUNCTION(BlueprintCallable)
	FName GetItemName();

	UFUNCTION(BlueprintCallable)
	uint8 GetItemCount();

	UFUNCTION(BlueprintCallable)
	void SetMoving(bool bMoving);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Velocity;
};
