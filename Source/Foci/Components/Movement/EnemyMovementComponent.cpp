#include "EnemyMovementComponent.h"

#include "Foci/Actors/Enemy.h"

#include "GenericPlatform/GenericPlatformMath.h" 
#include "Components/CapsuleComponent.h"

void UEnemyMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	PhysMovement(DeltaTime);
}

void UEnemyMovementComponent::SetUpdatedComponent(USceneComponent* Component)
{
	Super::SetUpdatedComponent(Component);

	EnemyOwner = Cast<AEnemy>(PawnOwner);
}

void UEnemyMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	SetDefaultMovementMode();
}

void UEnemyMovementComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UEnemyMovementComponent, MaxFloorWalkableAngle))
	{
		MaxFloorWalkableZ = FMath::Sin(FMath::DegreesToRadians(MaxFloorWalkableAngle));
	}
}

void UEnemyMovementComponent::HandleBlockingImpact(FHitResult ImpactHitResult)
{
	if (ImpactHitResult.IsValidBlockingHit())
	{
		Velocity -= Velocity.Dot(ImpactHitResult.ImpactNormal) * ImpactHitResult.ImpactNormal;
	}

	if (ImpactHitResult.ImpactNormal.Z >= MaxFloorWalkableZ)
	{
		MovementMode = EEnemyMovementMode::MOVE_Walking;
		Basis = ImpactHitResult.GetComponent();
	}
}

void UEnemyMovementComponent::PhysMovement(float DeltaTime)
{
	switch (MovementMode)
	{
		case(EEnemyMovementMode::MOVE_None):
			break;

		case(EEnemyMovementMode::MOVE_Walking):
			PhysWalking(DeltaTime);
			break;

		case(EEnemyMovementMode::MOVE_Falling):
			PhysFalling(DeltaTime);
			break;
	}
}

void UEnemyMovementComponent::PhysFalling(float DeltaTime)
{
	Velocity += FVector::UnitZ() * GetWorld()->GetGravityZ() * DeltaTime;
	FHitResult MoveHitResult;
	SafeMoveUpdatedComponent(Velocity * DeltaTime, UpdatedComponent->GetComponentRotation(), true, MoveHitResult);

	if (MoveHitResult.IsValidBlockingHit())
	{
		HandleBlockingImpact(MoveHitResult);
	}
}

void UEnemyMovementComponent::PhysWalking(float DeltaTime)
{
	const FVector Normal2D = Velocity.GetSafeNormal2D();
	const FVector PlanarVelocity = FVector(Velocity.X, Velocity.Y, 0.0f);
	FVector FrictionDelta = Normal2D * Friction;
	if (FrictionDelta.SquaredLength() > PlanarVelocity.SquaredLength())
	{
		FrictionDelta = -PlanarVelocity;
	}
	Velocity -= FrictionDelta;
	Velocity += Acceleration * ConsumeInputVector();
	Velocity = Velocity.GetClampedToMaxSize2D(MaxSpeed);

	DrawDebugDirectionalArrow(GetWorld(), PawnOwner->GetActorLocation(), PawnOwner->GetActorLocation() + BasisNormalRotator.RotateVector(Velocity), 25.0f, FColor::Red, false, DeltaTime * 2.0f);

	FHitResult MoveHitResult;
	SafeMoveUpdatedComponent(BasisNormalRotator.RotateVector(Velocity * DeltaTime), UpdatedComponent->GetComponentRotation(), true, MoveHitResult);
	if (MoveHitResult.IsValidBlockingHit())
	{
		HandleBlockingImpact(MoveHitResult);
	}
	SetDefaultMovementMode();
}

void UEnemyMovementComponent::SetDefaultMovementMode()
{
	if (SnapToFloor())
	{
		MovementMode = EEnemyMovementMode::MOVE_Walking;
		return;
	}
	MovementMode = EEnemyMovementMode::MOVE_Falling;
}

bool UEnemyMovementComponent::FindFloor(FHitResult& OutHitResult) const
{
	if (!GetWorld())
	{
		return false;
	}
	const UCapsuleComponent* Capsule = EnemyOwner->GetCapsule();
	const FVector CapsuleLocation = Capsule->GetComponentLocation();
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(PawnOwner);

	if (GetWorld()->SweepSingleByChannel(OutHitResult, CapsuleLocation, CapsuleLocation - FVector::UpVector * FloorSnapDistance,
		Capsule->GetComponentQuat(), Capsule->GetCollisionObjectType(), Capsule->GetCollisionShape(1.0f), CollisionQueryParams)
		&& OutHitResult.ImpactNormal.Z >= MaxFloorWalkableZ)
	{
			return true;
	}
	return false;
}

bool UEnemyMovementComponent::SnapToFloor()
{
	FHitResult HitResult;
	const bool bFoundFloor = FindFloor(HitResult);

	if (bFoundFloor)
	{
		Basis = HitResult.GetComponent();
		// Aka: If they aren't nearly equal
		if (!(BasisNormal - HitResult.ImpactNormal).IsNearlyZero())
		{
			BasisNormal = HitResult.ImpactNormal;
			BasisNormalRotator = FRotator(FMath::RadiansToDegrees(FGenericPlatformMath::Asin(-BasisNormal.X)),
				0.0f,
				FMath::RadiansToDegrees(FGenericPlatformMath::Asin(-BasisNormal.Y)));
		}
	}

	return bFoundFloor;
}

bool UEnemyMovementComponent::IsFalling() const
{
	return MovementMode == EEnemyMovementMode::MOVE_Falling;
}
