#include "PushableBlock.h"

#include "Components/BoxComponent.h"

#include "Foci/Actors/Buttons/FloorButton.h"

APushableBlock::APushableBlock()
{
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Block"));
	RootComponent = Box;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	StaticMesh->SetupAttachment(Box);
}

void APushableBlock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APushableBlock::Push(FVector Delta, AActor* Source, FHitResult& HitResult)
{
	const FVector StartLocation = Box->GetComponentLocation();
	const FVector EndLocation = StartLocation + Delta;
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(this);
	CollisionQueryParams.AddIgnoredActor(Source);

	GetWorld()->SweepSingleByProfile(HitResult, StartLocation, EndLocation, Box->GetComponentQuat(), Box->GetCollisionProfileName(), Box->GetCollisionShape(1.0f), CollisionQueryParams);
	
	if (HitResult.bBlockingHit)
	{
		if (AFloorButton* FloorButton = Cast<AFloorButton>(HitResult.GetActor()))
		{
			FloorButton->Trigger(Source);
			SnapToButton(FloorButton, Delta);
		}
		else
		{
			AddActorWorldOffset(HitResult.Location - StartLocation);
		}
	}
}

UBoxComponent* APushableBlock::GetBlockComponent() const
{
	return Box;
}

bool APushableBlock::SnapToFloor()
{
	return false;
}

bool APushableBlock::SnapToButton(AFloorButton* FloorButton, FVector Delta)
{
	return false;
}
