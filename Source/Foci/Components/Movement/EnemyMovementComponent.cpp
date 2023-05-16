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
	if (!RequestedVelocity.IsNearlyZero())
	{
		Velocity = RequestedVelocity.GetSafeNormal2D() * MaxSpeed;
	}
	Velocity -= FrictionDelta;
	Velocity += Acceleration * ConsumeInputVector();
	Velocity = Velocity.GetClampedToMaxSize2D(MaxSpeed);

	// DrawDebugDirectionalArrow(GetWorld(), PawnOwner->GetActorLocation(), PawnOwner->GetActorLocation() + Velocity, 25.0f, FColor::Red, false, DeltaTime * 2.0f);
	// DrawDebugDirectionalArrow(GetWorld(), PawnOwner->GetActorLocation(), PawnOwner->GetActorLocation() + BasisNormal * 50.0f, 50.0f, FColor::Blue, false, 10.0f);

	FHitResult MoveHitResult;
	const FVector DeltaLocation = Velocity * DeltaTime;
	// const FVector DeltaLocation = RequestedVelocity.GetClampedToMaxSize(MaxSpeed) * DeltaTime;
	const float FloorDotDelta = (BasisNormal | DeltaLocation);
	FVector RampMovement(DeltaLocation.X, DeltaLocation.Y, -FloorDotDelta / BasisNormal.Z);

	SafeMoveUpdatedComponent(RampMovement, UpdatedComponent->GetComponentRotation(), true, MoveHitResult);

	UCapsuleComponent* CapsuleComponent = EnemyOwner->GetCapsule();
	if (MoveHitResult.IsValidBlockingHit())
	{
		const FVector DesiredLocation = CapsuleComponent->GetComponentLocation() + DeltaLocation;
		FHitResult StepUpHitResult;
		const float StepUpHeight = 5.0f;
		FCollisionQueryParams CollisionQueryParams;
		CollisionQueryParams.AddIgnoredActor(PawnOwner);

		GetWorld()->SweepSingleByChannel(StepUpHitResult, DesiredLocation + FVector::UpVector * StepUpHeight, DesiredLocation, CapsuleComponent->GetComponentQuat(),
			CapsuleComponent->GetCollisionObjectType(), CapsuleComponent->GetCollisionShape(1.0f), CollisionQueryParams);
		if (StepUpHitResult.IsValidBlockingHit())
		{
			const FVector StepUpDelta = StepUpHitResult.Location - CapsuleComponent->GetComponentLocation();
			PawnOwner->AddActorWorldOffset(StepUpDelta);
		}

		HandleBlockingImpact(MoveHitResult);
	}
	SetDefaultMovementMode();
	UpdateRotation(DeltaTime);
}

void UEnemyMovementComponent::UpdateRotation(float DeltaTime)
{
	const FVector Normal2D = Velocity.GetSafeNormal2D();

	if (Normal2D.IsNearlyZero())
	{
		return;
	}
	FRotator CurrentRotation = PawnOwner->GetActorRotation();
	const float DesiredYaw = FMath::RadiansToDegrees(FMath::Atan2(Normal2D.Y, Normal2D.X));
	// const float DeltaYaw = FMath::FindDeltaAngleDegrees(DesiredYaw, CurrentRotation.Yaw);
	const float DeltaYaw = FMath::FInterpConstantTo(CurrentRotation.Yaw, DesiredYaw, DeltaTime, RotationSpeed);
	FRotator DesiredRotation = CurrentRotation;
	DesiredRotation.Yaw = DesiredYaw;
	FRotator NewRotation = FMath::RInterpConstantTo(CurrentRotation, DesiredRotation, DeltaTime, RotationSpeed);
	PawnOwner->SetActorRotation(NewRotation);
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

	const bool bValidHit = GetWorld()->SweepSingleByChannel(OutHitResult, CapsuleLocation, CapsuleLocation - FVector::UpVector * FloorSnapDistance,
		Capsule->GetComponentQuat(), Capsule->GetCollisionObjectType(), Capsule->GetCollisionShape(1.0f), CollisionQueryParams);

	// const bool bValidHit = GetWorld()->LineTraceSingleByChannel(OutHitResult, CapsuleLocation, CapsuleLocation - FVector::UpVector * (FloorSnapDistance + Capsule->GetScaledCapsuleHalfHeight()),
		// Capsule->GetCollisionObjectType(), CollisionQueryParams);

	if (bValidHit && OutHitResult.ImpactNormal.Z >= MaxFloorWalkableZ)
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
			/* BasisNormalRotator = FRotator(FMath::RadiansToDegrees(FGenericPlatformMath::Asin(-BasisNormal.X)),
				0.0f,
				FMath::RadiansToDegrees(FGenericPlatformMath::Asin(-BasisNormal.Y))); */
		}
	}

	return bFoundFloor;
}

void UEnemyMovementComponent::RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed)
{
	// UE_LOG(LogTemp, Display, TEXT("DirectMove '%s' requested."), *MoveVelocity.ToString())
	RequestedVelocity = MoveVelocity;
}

void UEnemyMovementComponent::RequestPathMove(const FVector& MoveInput)
{
	// UE_LOG(LogTemp, Display, TEXT("PathMove '%s' requested."), *MoveInput.ToString())
	Super::RequestPathMove(MoveInput);
}

bool UEnemyMovementComponent::CanStartPathFollowing() const
{
	return true;
}

bool UEnemyMovementComponent::CanStopPathFollowing() const
{
	return !IsFalling();
}

void UEnemyMovementComponent::StopActiveMovement()
{
	RequestedVelocity = FVector::Zero();
}

bool UEnemyMovementComponent::IsFalling() const
{
	return MovementMode == EEnemyMovementMode::MOVE_Falling;
}
