// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MarleMovementComponent.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EClimbingSurfaceType : uint8
{
	None,
	Ledge,
	Ladder,
	Wall
};

UCLASS()
class FOCI_API UMarleMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	/// How far away from the character horizontally we will check for available ledges when colliding with a surface
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Climbing|Ledges", meta = (AllowPrivateAccess = "true", ClampMin = 0.0f))
	float LedgeHorizontalReach = 60.0f;

	/// How far up and down a ledge can be from the character's hands and still be grabbed
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Climbing|Ledges", meta = (AllowPrivateAccess = "true", ClampMin = 0.0f))
	float LedgeVerticalReach = 20.0f;

	/// How far the character can jump off the ground to reach a ledge
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Climbing|Ledges", meta = (AllowPrivateAccess = "true", ClampMin = 0.0f))
	float LedgeJumpHeight = 80.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Climbing|Ledges", meta = (AllowPrivateAccess = "true", ClampMin = 0.0f))
	float ClimbingSpeed = 200.0f;

	/// The position of the character's hands, 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Climbing, meta = (AllowPrivateAccess = "true"))
	FVector ClimbOffset;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Climbing, meta = (AllowPrivateAccess = "true"))
	float MantleSpeed = 500.0f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = Climbing, Transient, meta = (AllowPrivateAccess = "true"))
	FVector ClimbSnapLocation;

	/// How long a player must press into a ledge to attempt to climb up it 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Climbing|Ledges", Transient, meta = (AllowPrivateAccess = "true"))
	float LedgeClimbRequiredHoldTime = 0.5f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Climbing|Ladder", Transient, meta = (AllowPrivateAccess = "true"))
	float InputVelocity = 0.0f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Climbing|Ladder", Transient, meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<class ALadder> LadderBase;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Climbing|Ladder", Transient, meta = (AllowPrivateAccess = "true"))
	float DistanceAlongLadder = 0.0f;


	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Tethered, Transient, meta = (AllowPrivateAccess = "true"))
	float TetherVelocity = 750.0f;


protected:
	virtual bool MoveUpdatedComponentImpl(const FVector& Delta, const FQuat& NewRotation, bool bSweep, FHitResult* OutHit, ETeleportType Teleport) override;

	virtual FRotator ComputeOrientToMovementRotation(const FRotator& CurrentRotation, float DeltaTime, FRotator& DeltaRotation) const override;

	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;

	virtual void PhysWalking(float DeltaTime, int32 Iterations) override;

	virtual void PerformMovement(float DeltaTime) override;

	virtual void ControlledCharacterMove(const FVector& InputVector, float DeltaSeconds) override;

	void CapsuleHit(UPrimitiveComponent* OverlappedComponent, UPrimitiveComponent* OtherComp, const FHitResult& SweepResult);

	bool CanGrabLedge(UPrimitiveComponent* CapsuleComponent, UPrimitiveComponent* LedgeComponent, const FHitResult& LedgeSweepResult, FVector& LedgeLocationOut);


public:
	virtual FRotator GetDeltaRotation(float DeltaTime) const override;

	UFUNCTION(BlueprintCallable)
	void FinishMantling();

	void JumpAction();

	void GrabLadder(class ALadder* Ladder);

	bool UsingDirectInput() const;

	EClimbingSurfaceType GetClimbingSurfaceType() const;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTetherBroken);
	FOnTetherBroken OnTetherBroken;

	void ActivateTether(FVector Location);

	void GrabBlock(class APushableBlock* Block);

	void ReleaseBlock();

	float GetInputVelocity() const;


private:
	bool IsMantling() const;

	void GrabLedge(UPrimitiveComponent* LedgeComponent, const FVector& WallNormal, const FVector& LedgeLocation);

	// FVector GetLedgeMovement(const float DeltaTime) const;

	FVector GetGrabLocation() const;

	FVector GetFloorPositionFront() const;

	void ClimbLedge();

	void ClimbLadder(FVector InputVector, float DeltaTime);

	void MoveBlock(FVector InputVector, float DeltaTime);


	void PhysClimbing(float DeltaTime, int32 Iterations);

	void PhysLadder(float DeltaTime, int32 Iterations);

	void PhysMantling(float DeltaTime, int32 Iterations);

	// Pull the player to the object
	void PhysTethered(float DeltaTime, int32 Iterations);

	void PhysPulling(float DeltaTime, int32 Iterations);

	void ReleaseLedge();

	void ReleaseLadder();

	bool bMantling = false;

	EClimbingSurfaceType ClimbingSurfaceType = EClimbingSurfaceType::None;

	float LedgeClimbTimeHeld = 0.0f;

	bool bPressingIntoWall = false;

	FVector TetherDestination;

	TWeakObjectPtr<class APushableBlock> GrabbedBlock;
};
