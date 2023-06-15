// Copyright 2023 Stephen Melnick

#include "MarleMovementComponent.h"

#include "GameFramework/Character.h"
#include "VisualLogger/VisualLogger.h"

#include "Ladder.h"
#include "FociCharacter.h"
#include "Foci/Foci.h"

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

////////////////////////////////////////////////////////////////////////////////////////////
// Phys Movement
void UMarleMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	Super::PhysCustom(DeltaTime, Iterations);

	switch (CustomMovementMode)
	{
		case 0:
			PhysClimbing(DeltaTime, Iterations);
			break;

		case 1:
			PhysTethered(DeltaTime, Iterations);
			break;

		case 2:
			PhysPulling(DeltaTime, Iterations);
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
	const FVector NewLocation = GetGrabLocation() + LadderBase->GetActorUpVector() * DeltaTime * InputVelocity * 200.0f;
	DistanceAlongLadder = LadderBase->GetGrabDistance(NewLocation);
	ClimbSnapLocation = LadderBase->GetGrabLocation(DistanceAlongLadder) - UpdatedComponent->GetComponentRotation().RotateVector(ClimbOffset);
	FHitResult HitResult;
	UpdatedComponent->SetWorldLocation(ClimbSnapLocation, true, &HitResult);
	if (HitResult.bBlockingHit && InputVelocity < 0.0f)
	{
		UE_LOG(LogTemp, Display, TEXT("Hit obstacle while climbing."))
			ReleaseLedge();
		return;
	}
	if (LadderBase->IsTop(DistanceAlongLadder) && InputVelocity > 0.0f)
	{
		UE_LOG(LogTemp, Display, TEXT("Reached end of ladder"))
			ReleaseLadder();

		ACharacter* Owner = CharacterOwner.Get();
		AFociCharacter* FociCharacter = Cast<AFociCharacter>(Owner);
		FociCharacter->Mantle(ClimbSnapLocation);
		return;
	}
	if (LadderBase->IsBottom(DistanceAlongLadder) && InputVelocity < 0.0f)
	{
		ReleaseLadder();
		return;
	}
}

void UMarleMovementComponent::PhysMantling(float DeltaTime, int32 Iterations)
{
	if (bMantling)
	{
		ApplyRootMotionToVelocity(DeltaTime);
		MoveUpdatedComponent(Velocity * DeltaTime, UpdatedComponent->GetComponentRotation(), false);
		return;
	}
	//FVector DeltaLocation = GetLedgeMovement(DeltaTime);
	FVector DeltaLocation = ClimbSnapLocation - UpdatedComponent->GetComponentLocation();
	if (DeltaLocation.SizeSquared() > FMath::Square(MantleSpeed * DeltaTime))
	{
		DeltaLocation.Normalize();
		DeltaLocation *= MantleSpeed * DeltaTime;
	}
	FHitResult HitResult;
	SafeMoveUpdatedComponent(DeltaLocation, UpdatedComponent->GetComponentRotation(), false, HitResult);
}

// Move along the tether, but break if we hit something
void UMarleMovementComponent::PhysTethered(float DeltaTime, int32 Iterations)
{
	const FVector DeltaDestination = TetherDestination - UpdatedComponent->GetComponentLocation();
	float DistanceMovedThisTick = TetherVelocity * DeltaTime;

	const float DeltaLengthSquared = DeltaDestination.SquaredLength();
	const float DistanceMovedThisTickSquared = DistanceMovedThisTick * DistanceMovedThisTick;
	// If we are trying to move further than our actual separation, just use our separation as the move
	FVector RequestedMovement;
	bool bLastMove = false;
	if (DistanceMovedThisTickSquared > DeltaLengthSquared)
	{
		bLastMove = true;
		RequestedMovement = DeltaDestination;
	}
	else
	{
		RequestedMovement = DistanceMovedThisTick * DeltaDestination.GetSafeNormal();
	}
	FHitResult TetherMoveHitResult;
	SafeMoveUpdatedComponent(RequestedMovement, UpdatedComponent->GetComponentRotation(), true, TetherMoveHitResult);
	if (TetherMoveHitResult.bBlockingHit || bLastMove)
	{
		// TODO: Implement more elegant solution? Currently split between the hookshot's code and here:
		// The hookshot disables it on hit, and we re-enable it here. This isn't really safe
		if (bLastMove)
		{
			UE_LOG(LogWeaponSystem, Display, TEXT("Hookshot tether broken, player reached destination."))
		}
		else
		{
			UE_LOG(LogWeaponSystem, Display, TEXT("Hookshot tether broken, player collided with object before reaching destination."))
		}
		Cast<AFociCharacter>(CharacterOwner)->SetInputEnabled(true);
		SetDefaultMovementMode();
		OnTetherBroken.Broadcast();
	}
}

void UMarleMovementComponent::PhysPulling(float DeltaTime, int32 Iterations)
{
	if (InputVelocity == 0.0f)
	{
		return;
	}
	if (!GrabbedBlock.IsValid())
	{
		return;
	}
	const float RelativeVelocity = InputVelocity * DeltaTime * 100.0f;
	// TODO: Use rootmotion here instead?
	const FVector InitialMovement = PawnOwner->GetActorForwardVector() * RelativeVelocity;

	// We're pushing forward, so the block is going to move before the character.
	if (InputVelocity > 0.0f)
	{
		FHitResult PushHitResult;
		const FVector BlockInitialLocation = GrabbedBlock->GetComponentLocation();
		GrabbedBlock->MoveComponent(InitialMovement, GrabbedBlock->GetComponentRotation(), true, &PushHitResult);
		const FVector BlockDelta = GrabbedBlock->GetComponentLocation() - BlockInitialLocation;

		FHitResult PushHitResult2;
		SafeMoveUpdatedComponent(BlockDelta, UpdatedComponent->GetComponentRotation(), false, PushHitResult2);
		return;
	}

	// We're pulling the block instead, so move the character first
	FHitResult PullHitResult;
	const FVector PlayerInitialLocation = UpdatedComponent->GetComponentLocation();
	SafeMoveUpdatedComponent(InitialMovement, UpdatedComponent->GetComponentRotation(), true, PullHitResult);

	const FVector CharacterDelta = UpdatedComponent->GetComponentLocation() - PlayerInitialLocation;
	GrabbedBlock->MoveComponent(CharacterDelta, GrabbedBlock->GetComponentRotation(), false);
}
////////////////////////////////////////////////////////////////////////////////////////////


void UMarleMovementComponent::PerformMovement(float DeltaTime)
{
	Super::PerformMovement(DeltaTime);
}

void UMarleMovementComponent::ControlledCharacterMove(const FVector& InputVector, float DeltaSeconds)
{
	// Controlled Character Move clears the inputvector, so we capture it here for our purposes
	if (MovementMode == EMovementMode::MOVE_Custom)
	{
		// MovementMode Climbing
		if (CustomMovementMode == 0)
		{
			if (ClimbingSurfaceType == EClimbingSurfaceType::Ladder)
			{
				ClimbLadder(InputVector.GetSafeNormal(), DeltaSeconds);
			}
		}

		// MovementMode Pulling
		if (CustomMovementMode == 2)
		{
			MoveBlock(InputVector.GetSafeNormal(), DeltaSeconds);
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

void UMarleMovementComponent::FinishMantling()
{
	bMantling = false;
	// UpdatedComponent->SetWorldLocation(ClimbSnapLocation);
	SetMovementMode(MOVE_Falling);
	Velocity = FVector::ZeroVector;
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
	if (LedgeComponent->IsAnySimulatingPhysics() || LedgeComponent->GetOwner()->IsA<AFociCharacter>())
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

void UMarleMovementComponent::ActivateTether(FVector Location)
{
	// 1 represents the 'Tethered' movement mode -- it's not ideal to represent it this way, but it fits with the existing modes easily
	SetMovementMode(EMovementMode::MOVE_Custom, 1);
	TetherDestination = Location;
}

void UMarleMovementComponent::GrabBlock(AActor* Block)
{
	// Please note, the "Block" is just terminology. It doesn't have to be a box shape.
	UPrimitiveComponent* BlockComponent = Cast<UPrimitiveComponent>(Block->GetRootComponent());
	if (!BlockComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Player attempted to grab a block object '%s', but the root component was not of a type derived from UPrimitiveComponent."), *Block->GetFName().ToString())
		return;
	}	
	// Do a line trace to find the surface normal of our block
	FHitResult GrabNormalSweepResult;
	const FVector SweepStart = PawnOwner->GetActorLocation();
	const FVector SweepEnd = SweepStart + PawnOwner->GetActorForwardVector() * 100.0f;
	FCollisionQueryParams CollisionQueryParams;

	BlockComponent->LineTraceComponent(GrabNormalSweepResult, SweepStart, SweepEnd, CollisionQueryParams);
	DrawDebugDirectionalArrow(GetWorld(), GrabNormalSweepResult.TraceStart, GrabNormalSweepResult.TraceEnd, 5.0f, FColor::Red, false, 5.0f);
	DrawDebugDirectionalArrow(GetWorld(), GrabNormalSweepResult.ImpactPoint, GrabNormalSweepResult.ImpactPoint + GrabNormalSweepResult.ImpactNormal * 50.0f, 5.0f, FColor::Blue, false, 5.0f);

	// This type of line trace doesn't work with bBlockingHit, so this is the simplest way to determine if we've hit the block or not.
	if (GrabNormalSweepResult.Time >= 1.0f)
	{
		return;
	}

	SetMovementMode(EMovementMode::MOVE_Custom, 2);

	bOrientRotationToMovement = false;
	bUseControllerDesiredRotation = false;

	const FVector Normal2D = GrabNormalSweepResult.ImpactNormal.GetSafeNormal2D();
	const float RotationYaw = FMath::RadiansToDegrees(FMath::Atan2(-Normal2D.Y, -Normal2D.X));
	FRotator PawnRotation = PawnOwner->GetActorRotation();
	PawnRotation.Yaw = RotationYaw;
	PawnOwner->SetActorRotation(PawnRotation);

	GrabbedBlock = BlockComponent;

	// Move the character into the block, and let the sweep take care of it
	MoveUpdatedComponent(Normal2D * -50.0f, UpdatedComponent->GetComponentRotation(), true);
}

void UMarleMovementComponent::ReleaseBlock()
{
	GrabbedBlock = nullptr;

	bOrientRotationToMovement = true;
	bUseControllerDesiredRotation = true;

	SetDefaultMovementMode();
}


void UMarleMovementComponent::GrabLadder(ALadder* Ladder)
{
	UE_LOG(LogTemp, Display, TEXT("Grabbed Ladder"))
	SetMovementMode(EMovementMode::MOVE_Custom, 0);
	ClimbingSurfaceType = EClimbingSurfaceType::Ladder;
	LadderBase = Ladder;
	bMantling = false;

	const FVector HandLocation = GetGrabLocation();
	DistanceAlongLadder = LadderBase->GetGrabDistance(HandLocation);
	ClimbSnapLocation = LadderBase->GetGrabLocation(DistanceAlongLadder) - UpdatedComponent->GetComponentRotation().RotateVector(ClimbOffset);
	UpdatedComponent->SetWorldRotation(LadderBase->GetGrabRotation(HandLocation));
	UpdatedComponent->SetWorldLocation(ClimbSnapLocation);
}

bool UMarleMovementComponent::UsingDirectInput() const
{
	return MovementMode == EMovementMode::MOVE_Custom;
}

bool UMarleMovementComponent::IsMantling() const
{
	return bMantling;
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

void UMarleMovementComponent::ClimbLedge()
{
	if (IsMantling())
	{
		return;
	}
	bMantling = true;
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

	ACharacter* Owner = CharacterOwner.Get();
	AFociCharacter* FociCharacter = Cast<AFociCharacter>(Owner);
	FociCharacter->Mantle(ClimbSnapLocation);
}

void UMarleMovementComponent::ClimbLadder(FVector InputVector, float DeltaTime)
{
	// The input is in 3D space -- X is forward in Unreal
	InputVelocity = InputVector.X;
}

void UMarleMovementComponent::MoveBlock(FVector InputVector, float DeltaTime)
{
	// This is probably going to end up being different than the ClimbLadder function, so I'll leave it as a separate implementation
	InputVelocity = InputVector.X;
}

void UMarleMovementComponent::ReleaseLedge()
{ 
	UE_LOG(LogTemp, Display, TEXT("Released Ledge"))
	Velocity = FVector::ZeroVector;
	bMantling = false;
	SetMovementMode(MOVE_Falling);
	ClimbingSurfaceType = EClimbingSurfaceType::None;
}

void UMarleMovementComponent::ReleaseLadder()
{
	UE_LOG(LogTemp, Display, TEXT("Released Ladder"))
	ClimbSnapLocation = GetFloorPositionFront();
	LadderBase = nullptr;
	DistanceAlongLadder = 0.0f;
	bMantling = true;
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
