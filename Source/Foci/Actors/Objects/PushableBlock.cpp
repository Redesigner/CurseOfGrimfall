#include "PushableBlock.h"

#include "Components/BoxComponent.h"

#include "Foci/Actors/Buttons/FloorButton.h"

APushableBlock::APushableBlock()
{
	PrimaryActorTick.bCanEverTick = true;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Block"));
	RootComponent = Box;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	StaticMesh->SetupAttachment(Box);
}

void APushableBlock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!SnapToFloor())
	{
		Velocity += FVector(0.0f, 0.0f, GetWorld()->GetGravityZ() * DeltaTime);
	}
	const FVector Delta = Velocity * DeltaTime;

	FHitResult MoveHitResult;
	const FVector StartLocation = Box->GetComponentLocation();
	const FVector EndLocation = StartLocation + Delta;
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(this);
	if (LastUser.IsValid())
	{
		CollisionQueryParams.AddIgnoredActor(LastUser.Get());
	}

	GetWorld()->SweepSingleByProfile(MoveHitResult, StartLocation, EndLocation, Box->GetComponentQuat(), Box->GetCollisionProfileName(), Box->GetCollisionShape(1.0f), CollisionQueryParams);

	if (MoveHitResult.bBlockingHit)
	{
		AddActorWorldOffset(MoveHitResult.Location - StartLocation, false);
		return;
	}
	AddActorWorldOffset(Delta);
}

bool APushableBlock::Push(FVector Delta, AActor* Source, FHitResult& HitResult)
{
	LastUser = Source;
	const FVector StartLocation = Box->GetComponentLocation();
	const FVector EndLocation = StartLocation + Delta;
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(this);
	CollisionQueryParams.AddIgnoredActor(Source);

	GetWorld()->SweepSingleByProfile(HitResult, StartLocation, EndLocation, Box->GetComponentQuat(), Box->GetCollisionProfileName(), Box->GetCollisionShape(0.0f), CollisionQueryParams);
	
	if (HitResult.IsValidBlockingHit())
	{
		UE_LOG(LogTemp, Display, TEXT("Pushing block, but block hit object '%s'"), *HitResult.GetActor()->GetFName().ToString())
		if (AFloorButton* FloorButton = Cast<AFloorButton>(HitResult.GetActor()))
		{
			FloorButton->Trigger(Source);
			SnapToButton(FloorButton, Delta);
		}
		else
		{
			const FVector Delta = HitResult.Location - StartLocation;
			AddActorWorldOffset(Delta - Delta.GetSafeNormal());
		}
	}
	else
	{
		AddActorWorldOffset(Delta);
	}

	return SnapToFloor();
}

UBoxComponent* APushableBlock::GetBlockComponent() const
{
	return Box;
}

bool APushableBlock::SnapToFloor()
{
	const float SnapHeight = 10.0f;

	FHitResult FloorHitResult;
	const FVector StartLocation = Box->GetComponentLocation();
	const FVector EndLocation = StartLocation - FVector(0.0f, 0.0f, SnapHeight);
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(this);
	if (LastUser.IsValid())
	{
		CollisionQueryParams.AddIgnoredActor(LastUser.Get());
	}

	GetWorld()->SweepSingleByProfile(FloorHitResult, StartLocation, EndLocation, Box->GetComponentQuat(), Box->GetCollisionProfileName(), Box->GetCollisionShape(1.0f), CollisionQueryParams);
	
	if (FloorHitResult.bBlockingHit)
	{
		AddActorWorldOffset(FloorHitResult.Location - StartLocation, false);
		if (AFloorButton* FloorButton = Cast<AFloorButton>(FloorHitResult.GetActor()))
		{
			FloorButton->Trigger(this);
		}
		Velocity.Z = 0.0f;
		return true;
	}

	return false;
}

// This also moves the block over the button, if need be
bool APushableBlock::SnapToButton(AFloorButton* FloorButton, FVector Delta)
{
	const float SnapHeight = 250.0f;
	// This assumes that the move would've been successful, if not for the button being in the way
	FHitResult ButtonSnapResult;
	const FVector EndLocation = Box->GetComponentLocation() + Delta;
	const FVector StartLocation = EndLocation + FVector(0.0f, 0.0f, SnapHeight);
	
	UBoxComponent* ButtonSurface = FloorButton->GetSurface();

	ButtonSurface->SweepComponent(ButtonSnapResult, StartLocation, EndLocation, Box->GetComponentQuat(), Box->GetCollisionShape(1.0f));

	DrawDebugDirectionalArrow(GetWorld(), StartLocation, ButtonSnapResult.Location, 5.0f, FColor::Red, false, 5.0f, -1);
	DrawDebugDirectionalArrow(GetWorld(), EndLocation, ButtonSnapResult.Location, 5.0f, FColor::Blue, false, 5.0f, -1);

	if (ButtonSnapResult.bBlockingHit)
	{
		// AddActorWorldOffset(ButtonSnapResult.Location - Box->GetComponentLocation());
		Box->SetWorldLocation(ButtonSnapResult.Location);
		return true;
	}

	// The sweep failed! This can sometimes happen, so just move the block here
	// AddActorWorldOffset(Delta);
	Box->SetWorldLocation(ButtonSnapResult.Location);
	return false;
}
