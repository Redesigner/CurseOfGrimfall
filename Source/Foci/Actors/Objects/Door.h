#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Foci/InteractableInterface.h"

#include "Door.generated.h"

class AFociGameMode;

UCLASS()
class FOCI_API ADoor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = Components, meta = (AllowPrivateAccess = true))
	class UCameraComponent* Camera;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = Components, meta = (AllowPrivateAccess = true))
	class UStaticMeshComponent* DoorMesh;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = Components, meta = (AllowPrivateAccess = true))
	class USceneComponent* EnterLocation;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = Components, meta = (AllowPrivateAccess = true))
	class USceneComponent* ExitLocation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Door, meta = (AllowPrivateAccess = true))
	bool bDoorOpen = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Door, meta = (AllowPrivateAccess = true))
	FVector DoorOpenOffset;
	FVector DoorInitialOffset;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Door, meta = (AllowPrivateAccess = true))
	float DoorMovementSpeed = 200.0f;

	FTimerHandle DoorSequenceTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, meta = (AllowPrivateAccess = true))
	class USoundBase* MovingSound;

	AFociGameMode* GameMode;

public:
	ADoor();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void Interact(class AFociCharacter* Source) override;

private:
	void FlipDoor(FVector PlayerLocation);
};
