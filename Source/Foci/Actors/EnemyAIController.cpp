#include "EnemyAIController.h"

#include "Kismet/GameplayStatics.h" 

#include "Foci/Actors/Enemy.h"
#include "Foci/Components/Movement/EnemyMovementComponent.h"

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	EnemyPawn = Cast<AEnemy>(InPawn);
}

void AEnemyAIController::SetTarget(APawn* Target)
{
	CurrentTarget = Target;
}

FVector AEnemyAIController::GetDestination() const
{
	if (!CurrentTarget.IsValid())
	{
		// if (CanSeeTarget(CurrentTarget.Get())
		return EnemyPawn->GetActorLocation();
	}

	const FVector TargetDelta = CurrentTarget->GetActorLocation() - EnemyPawn->GetActorLocation();
	const FVector TargetDirection = TargetDelta.GetSafeNormal2D();
	const float TargetDistanceSquared = TargetDelta.SquaredLength();
	float DesiredDistance = MaxDistance;

	float ModifiedMaxDistance = MaxDistance - 35.0f;

	if (TargetDistanceSquared < MaxDistance * MaxDistance)
	{
		// If the target is not closer than our min distance, we don't need to move
		if (TargetDistanceSquared >= ModifiedMaxDistance * ModifiedMaxDistance)
		{
			return EnemyPawn->GetActorLocation();
		}
		// otherwise, move so we are at least mindistance away from the target
		DesiredDistance = MinDistance;
	}
	const FVector TargetOffset = TargetDirection * DesiredDistance;
	return CurrentTarget->GetActorLocation() - TargetOffset;
}

bool AEnemyAIController::CanSeeTarget(APawn* Target) const
{
	if (!Target)
	{
		return false;
	}

	FVector EyePosition;
	FRotator EyeRotation;
	EnemyPawn->GetActorEyesViewPoint(EyePosition, EyeRotation);
	const FVector EyeNormal = EnemyPawn->GetActorForwardVector();

	// Currently, we are just looking directly at the target
	const FVector TargetDistance = Target->GetActorLocation() - EyePosition;

	if (TargetDistance.SquaredLength()  > ViewDistance * ViewDistance)
	{
		return false;
	}

	// Don't restrict cone based on up/down... so only use the 2D normal
	const FVector TargetNormal = TargetDistance.GetSafeNormal2D();
	const float AngleBetween = FMath::RadiansToDegrees(FMath::Acos(TargetNormal | EyeNormal));

	if (AngleBetween * 2.0f >= ConeViewingAngle)
	{
		/* UE_LOG(LogTemp, Display, TEXT("Target '%s' cannot be seen by Enemy '%s'. Target is outside cone of vision. Angle: '%f'"),
			*Target->GetFName().ToString(), *EnemyPawn->GetFName().ToString(), AngleBetween) */
		// Target is outside our cone of vision
		return false;
	}

	FHitResult VisibleHitResult;
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(EnemyPawn.Get());
	CollisionQueryParams.AddIgnoredActor(Target);

	if (GetWorld()->LineTraceSingleByChannel(VisibleHitResult, EyePosition, Target->GetActorLocation(), ECC_Visibility, CollisionQueryParams))
	{
		// UE_LOG(LogTemp, Display, TEXT("Target '%s' cannot be seen by Enemy '%s'. Line of sight is obstructed."), *Target->GetFName().ToString(), *EnemyPawn->GetFName().ToString())
		return false;
	}
	return true;
}

void AEnemyAIController::SetStrafing(bool Value)
{
	bStrafing = Value;
	EnemyPawn->GetEnemyMovementComponent()->SetOrientRotationToMovement(Value);
}

bool AEnemyAIController::IsCloseToTarget(APawn* Target, float Distance) const
{
	const FVector Delta = Target->GetActorLocation() - EnemyPawn->GetActorLocation();
	float DistanceSquared = Delta.SquaredLength();

	return (Distance * Distance) >= DistanceSquared;
}

void AEnemyAIController::TryExecuteAttack()
{
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	if (TimerManager.IsTimerActive(AttackTimerHandle))
	{
		return;
	}
	EnemyPawn->Attack();
	TimerManager.SetTimer(AttackTimerHandle, AttackCooldown, false);
}

void AEnemyAIController::Tick(float DeltaTime)
{
	if (!CurrentTarget.IsValid())
	{
		APawn* Target = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if (CanSeeTarget(Target))
		{
			CurrentTarget = Target;
		}
	}
	Super::Tick(DeltaTime);
	FVector Destination = GetDestination();
	// DrawDebugSphere(GetWorld(), Destination, 25.0f, 4, FColor::Blue, false, 2.0f * DeltaTime);
	MoveTo(Destination); 

	if (CurrentTarget.IsValid())
	{
		const FVector Delta = CurrentTarget->GetActorLocation() - EnemyPawn->GetActorLocation();
		// UE_LOG(LogTemp, Display, TEXT("Target is %f from enemy"), Delta.Length())

		SetStrafing(!IsCloseToTarget(CurrentTarget.Get(), MaxDistance * 2.0f));
		SetControlRotation(Delta.ToOrientationRotator());
		if (IsCloseToTarget(CurrentTarget.Get(), 150.0f))
		{
			TryExecuteAttack();
		}
	}
}
