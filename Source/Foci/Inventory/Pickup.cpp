#include "Pickup.h"


#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h" 

#include "Foci/FociCharacter.h"

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
		bIsMoving = false;
		return;
	}
	FHitResult HitResult;
	Velocity += FVector::UnitZ() * GetWorld()->GetGravityZ() * DeltaTime * 2.0f;
	RootComponent->MoveComponent(Velocity * DeltaTime, RootComponent->GetComponentRotation(), true, &HitResult);
	if (HitResult.bBlockingHit)
	{
		Velocity -= Velocity.Dot(HitResult.ImpactNormal) * HitResult.ImpactNormal;
		// bIsMoving = false;
	}
}

bool APickup::Pickup(AFociCharacter* Character)
{
	if (!bActive)
	{
		return false;
	}
	bActive = false;
	AttachToActor(Character, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	AddActorWorldOffset(FVector(0.0f, 0.0f, DisplayHeight));
	bIsMoving = false;
	SetLifeSpan(DisplayTime);
	
	if (PickupSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), PickupSound);
	}
	OnPickup(Character);
	return true;
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
