#include "Pickup.h"

APickup::APickup(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
}

void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsMoving)
	{
		return;
	}

	if (Velocity.IsNearlyZero())
	{
		return;
	}
	FHitResult HitResult;
	Velocity += FVector::UnitZ() * GetWorld()->GetGravityZ() * DeltaTime;
	RootComponent->MoveComponent(Velocity * DeltaTime, RootComponent->GetComponentRotation(), true, &HitResult);
	if (HitResult.bBlockingHit)
	{
		Velocity = FVector::Zero();
		bIsMoving = false;
	}
}

FName APickup::GetItemName()
{
	return ItemName;
}

uint8 APickup::GetItemCount()
{
	return ItemCount;
}

void APickup::SetMoving(bool bMoving)
{
	bIsMoving = bMoving;
}
