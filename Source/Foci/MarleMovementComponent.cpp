// Copyright 2023 Stephen Melnick

#include "MarleMovementComponent.h"

#include "GameFramework/Character.h"
#include "VisualLogger/VisualLogger.h"

#include "Ladder.h"

FRotator UMarleMovementComponent::ComputeOrientToMovementRotation(const FRotator& CurrentRotation, float DeltaTime, FRotator& DeltaRotation) const
{
	if (IsFalling())
	{
		return CurrentRotation;
	}
	return Super::ComputeOrientToMovementRotation(CurrentRotation, DeltaTime, DeltaRotation);
}

bool UMarleMovementComponent::MoveUpdatedComponentImpl(const FVector& Delta, const FQuat& NewRotation, bool bSweep, FHitResult* OutHit, ETeleportType Teleport)
{
	const bool bResult = Super::MoveUpdatedComponentImpl(Delta, NewRotation, bSweep, OutHit, Teleport);
	if (!OutHit)
	{
		return bResult;
	}
	if (OutHit->IsValidBlockingHit())
	{
		UPrimitiveComponent* OwnerPrimitive = Cast<UPrimitiveComponent>(UpdatedComponent);
		if (!OwnerPrimitive)
		{
			return bResult;
		}
		CapsuleHit(OwnerPrimitive, OutHit->GetComponent(), *OutHit);
	}
	return bResult;
}

void UMarleMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	Super::PhysCustom(DeltaTime, Iterations);

	switch (CustomMovementMode)
	{
		case 0:
			PhysClimbing(DeltaTime, Iterations);
			break;
		default: break;
	}
}

void UMarleMovementComponent::PhysWalking(float DeltaTime, int32 Iterations)
{
	Super::PhysWalking(DeltaTime, Iterations);
	
	if (bPressingIntoWall)
	{
		LedgeClimbTimeHeld += DeltaTime;
		bPressingIntoWall = false;
	}
	else
	{
		LedgeClimbTimeHeld = 0.0f;
	}
	if (LedgeClimbTimeHeld >= LedgeClimbRequiredHoldTime)
	{
		UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(UpdatedComponent);
		FHitResult ForwardSweepResult;
		const FVector StartLocation = UpdatedComponent->GetComponentLocation();
		const FVector EndLocation = StartLocation + UpdatedComponent->GetForwardVector() * LedgeHorizontalReach;
		FCollisionQueryParams CollisionQueryParams;
		CollisionQueryParams.AddIgnoredActor(UpdatedComponent->GetOwner());
		GetWorld()->LineTraceSingleByProfile(ForwardSweepResult, StartLocation, EndLocation, PrimitiveComponent->GetCollisionProfileName(), CollisionQueryParams);
		FVector GrabLocation;
		if (!ForwardSweepResult.GetComponent())
		{
			CharacterOwner->Jump();
			return;
		}
		if (CanGrabLedge(PrimitiveComponent, ForwardSweepResult.GetComponent(), ForwardSweepResult, GrabLocation))
		{
			GrabLedge(ForwardSweepResult.GetComponent(), ForwardSweepResult.ImpactNormal, GrabLocation);
		}
		else
		{
			LedgeClimbTimeHeld = 0.0f;
		}
	}
}

void UMarleMovementComponent::PerformMovement(float DeltaTime)
{
	Super::PerformMovement(DeltaTime);
}

void UMarleMovementComponent::ControlledCharacterMove(const FVector& InputVector, float DeltaSeconds)
{
	// Controlled Character Move clears the inputvector, so we capture it here for our purposes
	if (MovementMode == EMovementMode::MOVE_Custom && CustomMovementMode == 0)
	{
		if (ClimbingSurfaceType == EClimbingSurfaceType::Ladder)
		{
			ClimbLadder(InputVector.GetSafeNormal(), DeltaSeconds);
		}
	}
	Super::ControlledCharacterMove(InputVector, DeltaSeconds);
}

void UMarleMovementComponent::CapsuleHit(UPrimitiveComponent* OverlappedComponent, UPrimitiveComponent* OtherComp, const FHitResult& SweepResult)
{
	if (!(MovementMode == EMovementMode::MOVE_Walking || MovementMode == EMovementMode::MOVE_Falling))
	{
		return;
	}
	if (MovementMode == EMovementMode::MOVE_Walking)
	{
		const float VelocityAlongNormal = Velocity.Dot(SweepResult.ImpactNormal);
		if (VelocityAlongNormal >= -1000.0f)
		{
			bPressingIntoWall = true;
			return;
		}
		if (FMath::Abs(SweepResult.ImpactNormal.Z) >= 0.5f)
		{
			return;
		}
	}
	FVector LedgeGrabLocation;
	if (CanGrabLedge(OverlappedComponent, OtherComp, SweepResult, LedgeGrabLocation))
	{
		GrabLedge(OverlappedComponent, SweepResult.ImpactNormal, LedgeGrabLocation);
	}
	else if (LedgeGrabLocation.Z - UpdatedComponent->GetComponentLocation().Z <= 20.0f && MovementMode == EMovementMode::MOVE_Walking)
	{
		if (UpdatedComponent->GetForwardVector().Dot(-SweepResult.ImpactNormal) >= 0.75f)
		{
			CharacterOwner->Jump();
		}
	}
}

FRotator UMarleMovementComponent::GetDeltaRotation(float DeltaTime) const
{
	if (MovementMode != EMovementMode::MOVE_Custom)
	{
		return Super::GetDeltaRotation(DeltaTime);
	}

	switch (CustomMovementMode)
	{
		default:
			return FRotator::ZeroRotator;
		case 0:
			return FRotator::ZeroRotator;
	}
}

void UMarleMovementComponent::JumpAction()
{
	if (MovementMode == EMovementMode::MOVE_Custom && CustomMovementMode == 0)
	{
		if (ClimbingSurfaceType == EClimbingSurfaceType::Ledge)
		{
			ClimbLedge();
		}
		else
		{
			ReleaseLedge();
		}
	}
}

EClimbingSurfaceType UMarleMovementComponent::GetClimbingSurfaceType() const
{
	return ClimbingSurfaceType;
}

bool UMarleMovementComponent::CanGrabLedge(UPrimitiveComponent* CapsuleComponent, UPrimitiveComponent* LedgeComponent,
	const FHitResult& LedgeSweepResult, FVector& LedgeLocationOut)
{
	if (MovementMode == EMovementMode::MOVE_Custom && CustomMovementMode == 0)
	{
		// Don't allow ledge grabbing during climbing movement mode.
		return false;
	}
	// Don't climb on physics objects
	if (LedgeComponent->IsAnySimulatingPhysics())
	{
		return false;
	}
	const float WallFacingCoefficient = CapsuleComponent->GetForwardVector().Dot(-LedgeSweepResult.ImpactNormal);
	if (WallFacingCoefficient <= 0.75f)
	{
		UE_LOG(LogTemp, Display, TEXT("Attempting to climb ledge, but character is not facing it"))
		return false;
	}
	if (!FMath::IsNearlyZero(LedgeSweepResult.ImpactNormal.Z, 0.001))
	{
		return false;
	}
	FVector SweepStartLocation = CapsuleComponent->GetComponentLocation() - LedgeSweepResult.ImpactNormal * LedgeHorizontalReach + FVector::UpVector * LedgeVerticalReach;
	const FVector SweepEndLocation = SweepStartLocation - FVector::UpVector * LedgeVerticalReach * 2.0f;
	if (!IsFalling())
	{
		SweepStartLocation.Z += LedgeJumpHeight;
	}
	FHitResult SurfaceHitResult;
	const FCollisionQueryParams CollisionQueryParams;
	// Line trace down, to see if the object we just hit has a top surface we can grab
	if (!LedgeComponent->LineTraceComponent(SurfaceHitResult, SweepStartLocation, SweepEndLocation, CollisionQueryParams))
	{
		return false;
	}
	if (SurfaceHitResult.ImpactNormal.Z < 0.9f)
	{
		return false;
	}
	// Calculate where the two normals would intersect, using the top impact normal as our plane vector
	const FVector LedgeLocationVertical = SurfaceHitResult.ImpactPoint.Dot(SurfaceHitResult.ImpactNormal) * SurfaceHitResult.ImpactNormal;
	const FVector LedgeLocationHorizontal = LedgeSweepResult.ImpactPoint - (LedgeSweepResult.ImpactPoint.Dot(SurfaceHitResult.ImpactNormal) * SurfaceHitResult.ImpactNormal);
	LedgeLocationOut = LedgeLocationVertical + LedgeLocationHorizontal;
	bool WillForceIntoGround = false;
	if (IsFalling())
	{
		FHitResult GroundHitTest;
		const FVector GroundStartLocation = UpdatedComponent->GetComponentLocation();
		const FVector GroundEndLocation = FVector(GroundStartLocation.X, GroundStartLocation.Y, (LedgeLocationOut - UpdatedComponent->GetComponentRotation().RotateVector(ClimbOffset)).Z);
		const FCollisionShape& Capsule = CapsuleComponent->GetCollisionShape(-5.0f);
		FCollisionQueryParams GroundTestCollisionQueryParams;
		GroundTestCollisionQueryParams.AddIgnoredActor(CharacterOwner);
		GetWorld()->SweepSingleByChannel(GroundHitTest, GroundStartLocation, GroundEndLocation, UpdatedComponent->GetComponentQuat(), ECollisionChannel::ECC_WorldStatic, Capsule, GroundTestCollisionQueryParams);
		WillForceIntoGround = GroundHitTest.bBlockingHit;
		DrawDebugCapsule(GetWorld(), GroundStartLocation, Capsule.GetCapsuleHalfHeight(), Capsule.GetCapsuleRadius(), UpdatedComponent->GetComponentQuat(), FColor::Blue, false, 15.0f);
		DrawDebugCapsule(GetWorld(), GroundEndLocation, Capsule.GetCapsuleHalfHeight(), Capsule.GetCapsuleRadius(), UpdatedComponent->GetComponentQuat(), FColor::Blue, false, 15.0f);
		DrawDebugDirectionalArrow(GetWorld(), GroundStartLocation, GroundEndLocation, 5.0f, FColor::Blue, false, 15.0f);
		DrawDebugCapsule(GetWorld(), GroundHitTest.Location, Capsule.GetCapsuleHalfHeight(), Capsule.GetCapsuleRadius(), UpdatedComponent->GetComponentQuat(), FColor::Red, false, 15.0f);
	}
	else
	{
		WillForceIntoGround = LedgeLocationOut.Z - ClimbOffset.Z <= UpdatedComponent->GetComponentLocation().Z;
	}
	if (WillForceIntoGround)
	{
		UE_LOG(LogTemp, Display, TEXT("Ledge too low! Climbing onto this ledge will force the player through the floor."))
		return false;
	}
	if (SurfaceHitResult.bStartPenetrating)
	{
		UE_LOG(LogTemp, Display, TEXT("Ledge too high! Can't reach!"))
		UE_LOG(LogTemp, Display, TEXT("Ledge Z: %f, %s Z: %f"),
			LedgeLocationOut.Z,
			*UpdatedComponent.GetFName().ToString(),
			UpdatedComponent->GetComponentLocation().Z)
		return false;
	}
	return true;
}

bool UMarleMovementComponent::IsMantling() const
{
	return MantlingStatus != EMantlingStatus::NotMantling;
}

void UMarleMovementComponent::GrabLadder(ALadder* Ladder)
{
	UE_LOG(LogTemp, Display, TEXT("Grabbed Ladder"))
	SetMovementMode(EMovementMode::MOVE_Custom, 0);
	ClimbingSurfaceType = EClimbingSurfaceType::Ladder;
	LadderBase = Ladder;
	MantlingStatus = EMantlingStatus::NotMantling;

	const FVector HandLocation = GetGrabLocation();
	DistanceAlongLadder = LadderBase->GetGrabDistance(HandLocation);
	ClimbSnapLocation = LadderBase->GetGrabLocation(DistanceAlongLadder) - UpdatedComponent->GetComponentRotation().RotateVector(ClimbOffset);
	UpdatedComponent->SetWorldRotation(LadderBase->GetGrabRotation(HandLocation));
	UpdatedComponent->SetWorldLocation(ClimbSnapLocation);
}

bool UMarleMovementComponent::UseDirectInput() const
{
	return MovementMode == EMovementMode::MOVE_Custom;
}

void UMarleMovementComponent::GrabLedge(UPrimitiveComponent* LedgeComponent, const FVector& WallNormal, const FVector& LedgeLocation)
{
	Velocity = FVector::Zero();
	SetMovementMode(EMovementMode::MOVE_Custom, 0);

	const float AngleToWall = FMath::RadiansToDegrees(FMath::Atan2(-WallNormal.Y, -WallNormal.X));
	const FRotator LedgeFacingRotator = FRotator(0.0f, AngleToWall, 0.0f);
	const FVector NewLocation = LedgeLocation + LedgeFacingRotator.RotateVector(-ClimbOffset);

	UE_LOG(LogTemp, Display, TEXT("Grabbing ledge at %s, self location is at %s"), *LedgeLocation.ToString(), *UpdatedComponent->GetComponentLocation().ToString())

	UpdatedComponent->SetWorldRotation(LedgeFacingRotator);
	ClimbSnapLocation = NewLocation;
	ClimbingSurfaceType = EClimbingSurfaceType::Ledge;
	// UpdatedComponent->SetWorldLocation(NewLocation);
	// SetBase(LedgeComponent);
}

FVector UMarleMovementComponent::GetLedgeMovement(const float DeltaTime) const
{
	switch (MantlingStatus)
	{
	default:
		return ClimbSnapLocation - UpdatedComponent->GetComponentLocation();
	case EMantlingStatus::VerticalClimb:
		return FVector(0.0f, 0.0f, ClimbSnapLocation.Z - UpdatedComponent->GetComponentLocation().Z);
	}
}

void UMarleMovementComponent::ClimbLedge()
{
	if (IsMantling())
	{
		return;
	}
	MantlingStatus = EMantlingStatus::VerticalClimb;
	UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(UpdatedComponent);
	if (!PrimitiveComponent)
	{
		return;
	}
	const FCollisionShape& CollisionShape = PrimitiveComponent->GetCollisionShape(1.0f);
	FHitResult HitResult;
	const FVector SweepStartLocation = ClimbSnapLocation +
		UpdatedComponent->GetForwardVector() * CollisionShape.GetCapsuleRadius() * 2.0f +
		FVector::UpVector * (10.0f + CollisionShape.GetCapsuleHalfHeight() * 2.0f);

	const FVector SweepEndLocation = SweepStartLocation - FVector::UpVector * (10.0f + CollisionShape.GetCapsuleHalfHeight() * 2.0f);

	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredComponent(PrimitiveComponent);

	GetWorld()->SweepSingleByProfile(HitResult, SweepStartLocation, SweepEndLocation,
		UpdatedComponent->GetComponentQuat(), PrimitiveComponent->GetCollisionProfileName(), CollisionShape, CollisionQueryParams);

	ClimbSnapLocation = HitResult.Location;
	// DrawDebugDirectionalArrow(GetWorld(), SweepStartLocation, SweepEndLocation, 10.0f, FColor::Blue, false, 60.0f, 0U, 2.0f);
}

void UMarleMovementComponent::ClimbLadder(FVector InputVector, float DeltaTime)
{
	// The input is in 3D space -- X is forward in Unreal
	ClimbingVelocity = InputVector.X;
}

void UMarleMovementComponent::PhysClimbing(float DeltaTime, int32 Iterations)
{
	switch (ClimbingSurfaceType)
	{
	case EClimbingSurfaceType::Ledge:
		PhysMantling(DeltaTime, Iterations);
		break;
	case EClimbingSurfaceType::Ladder:
		PhysLadder(DeltaTime, Iterations);
		break;
	default:
		UE_LOG(LogTemp, Display, TEXT("Climbing, but current climbing surface type is not supported"))
	}
}

void UMarleMovementComponent::PhysLadder(float DeltaTime, int32 Iterations)
{
	if (IsMantling())
	{
		PhysMantling(DeltaTime, Iterations);
		return;
	}
	const FVector NewLocation = GetGrabLocation() + LadderBase->GetActorUpVector() * DeltaTime * ClimbingVelocity * 200.0f;
	DistanceAlongLadder = LadderBase->GetGrabDistance(NewLocation);
	ClimbSnapLocation = LadderBase->GetGrabLocation(DistanceAlongLadder) - UpdatedComponent->GetComponentRotation().RotateVector(ClimbOffset);
	FHitResult HitResult;
	UpdatedComponent->SetWorldLocation(ClimbSnapLocation, true, &HitResult);
	if (HitResult.bBlockingHit && ClimbingVelocity < 0.0f)
	{
		UE_LOG(LogTemp, Display, TEXT("Hit obstacle while climbing."))
		ReleaseLedge();
		return;
	}
	if (LadderBase->IsTop(DistanceAlongLadder) && ClimbingVelocity > 0.0f)
	{
		UE_LOG(LogTemp, Display, TEXT("Reached end of ladder"))
		ReleaseLadder();
		return;
	}
	if (LadderBase->IsBottom(DistanceAlongLadder) && ClimbingVelocity < 0.0f)
	{
		ReleaseLadder();
		return;
	}
}

void UMarleMovementComponent::PhysMantling(float DeltaTime, int32 Iterations)
{
	FVector DeltaLocation = GetLedgeMovement(DeltaTime);
	if (DeltaLocation.SizeSquared() > FMath::Square(MantleSpeed * DeltaTime))
	{
		DeltaLocation.Normalize();
		DeltaLocation *= MantleSpeed * DeltaTime;
	}
	else
	{
		// Move through our states
		switch (MantlingStatus)
		{
		case VerticalClimb:
			MantlingStatus = HorizontalClimb;
			break;
		case HorizontalClimb:
			UE_LOG(LogTemp, Display, TEXT("Mantling complete"))
			ReleaseLedge();
			break;
		default:
			break;
		}
	}
	FHitResult HitResult;
	SafeMoveUpdatedComponent(DeltaLocation, UpdatedComponent->GetComponentRotation(), false, HitResult);
}

void UMarleMovementComponent::ReleaseLedge()
{ 
	UE_LOG(LogTemp, Display, TEXT("Released Ledge"))
	Velocity = FVector::ZeroVector;
	MantlingStatus = NotMantling;
	SetMovementMode(MOVE_Falling);
	ClimbingSurfaceType = EClimbingSurfaceType::None;
}

void UMarleMovementComponent::ReleaseLadder()
{
	UE_LOG(LogTemp, Display, TEXT("Released Ladder"))
	ClimbSnapLocation = GetFloorPositionFront();
	LadderBase = nullptr;
	DistanceAlongLadder = 0.0f;
	MantlingStatus = EMantlingStatus::VerticalClimb;
}

FVector UMarleMovementComponent::GetGrabLocation() const
{
	return UpdatedComponent->GetComponentLocation() + UpdatedComponent->GetComponentRotation().RotateVector(ClimbOffset);
}

FVector UMarleMovementComponent::GetFloorPositionFront() const
{
	UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(UpdatedComponent);
	if (!PrimitiveComponent)
	{
		return FVector::ZeroVector;
	}
	const float VerticalReach = LedgeVerticalReach + 100.0f;
	const FVector SweepStartLocation = UpdatedComponent->GetComponentLocation() + UpdatedComponent->GetForwardVector() * LedgeHorizontalReach + FVector::UpVector * VerticalReach;
	const FVector SweepEndLocation = SweepStartLocation - FVector::UpVector * VerticalReach * 2.0f;
	FHitResult HitResult;
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(UpdatedComponent->GetOwner());
	GetWorld()->SweepSingleByProfile(HitResult, SweepStartLocation, SweepEndLocation, PrimitiveComponent->GetComponentQuat(), PrimitiveComponent->GetCollisionProfileName(), PrimitiveComponent->GetCollisionShape(1.0f), CollisionQueryParams);
	
	// FCollisionShape Capsule = PrimitiveComponent->GetCollisionShape();
	// DrawDebugCapsule(GetWorld(), SweepStartLocation, Capsule.GetCapsuleHalfHeight(), Capsule.GetCapsuleRadius(), UpdatedComponent->GetComponentQuat(), FColor::Blue, false, 15.0f);
	// DrawDebugCapsule(GetWorld(), SweepEndLocation, Capsule.GetCapsuleHalfHeight(), Capsule.GetCapsuleRadius(), UpdatedComponent->GetComponentQuat(), FColor::Blue, false, 15.0f);
	
	if (!HitResult.bBlockingHit)
	{
		return UpdatedComponent->GetComponentLocation();
	}
	return HitResult.Location;
}
