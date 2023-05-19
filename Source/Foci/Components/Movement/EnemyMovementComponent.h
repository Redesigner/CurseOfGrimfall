#pragma once

#include "CoreMinimal.h"

#include "GameFramework/PawnMovementComponent.h"
#include "EnemyMovementComponent.generated.h"

UENUM(BlueprintType)
enum class EEnemyMovementMode : uint8
{
	MOVE_None,
	MOVE_Walking,
	MOVE_Falling
};

UCLASS()
class FOCI_API UEnemyMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;


	virtual void HandleBlockingImpact(FHitResult ImpactHitResult);

	virtual void PhysMovement(float DeltaTime);

	virtual void PhysFalling(float DeltaTime);

	virtual void PhysWalking(float DeltaTime);

	void UpdateRotation(float DeltaTime);

	FVector ConsumeImpulses();

	virtual void SetDefaultMovementMode();

	virtual bool FindFloor(FHitResult& OutHitResult) const;

	virtual bool SnapToFloor();

	// Navigation Movement component interface
	virtual void RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed) override;
	virtual void RequestPathMove(const FVector& MoveInput) override;
	virtual bool CanStartPathFollowing() const override;
	virtual bool CanStopPathFollowing() const override;
	virtual void StopActiveMovement() override;


private:
	TWeakObjectPtr<class AEnemy> EnemyOwner;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Movement, meta = (AllowPrivateAccess = true))
	EEnemyMovementMode MovementMode;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Movement, meta = (AllowPrivateAccess = true))
	float MaxSpeed = 300.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Movement, meta = (AllowPrivateAccess = true))
	float Friction = 100.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Movement, meta = (AllowPrivateAccess = true))
	float Acceleration = 150.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement|Falling", meta = (AllowPrivateAccess = true))
	float FloorSnapDistance = 10.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement|Floor", meta = (AllowPrivateAccess = true, ClampMax = 90.0f, ClampMin = 0.0f))
	float MaxFloorWalkableAngle = 35.0f;
	float MaxFloorWalkableZ = 0.573576f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Movement|Floor", meta = (AllowPrivateAccess = true))
	TWeakObjectPtr<UPrimitiveComponent> Basis;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Movement|Floor", meta = (AllowPrivateAccess = true))
	FVector BasisNormal;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Rotation", meta = (AllowPrivateAccess = true))
	bool bOrientRotationToMovement = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Rotation", meta = (AllowPrivateAccess = true))
	float RotationSpeed = 30.0f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Movement|Pathfinding", meta = (AllowPrivateAccess = true))
	FVector RequestedVelocity;

	FVector PendingImpulses;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement|Strafing", meta = (AllowPrivateAccess = true))
	float MaxStrafeSpeed = 200.0f;

public:
	virtual void SetUpdatedComponent(USceneComponent* Component) override;

	UFUNCTION(BlueprintCallable)
	void AddImpulse(FVector Impulse);

	bool IsFalling() const;

	void SetOrientRotationToMovement(bool Value);

};
