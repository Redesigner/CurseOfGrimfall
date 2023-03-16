// Copyright 2023 Stephen Melnick
#include "Ladder.h"

#include "Components/SplineComponent.h"
#include "Components/BoxComponent.h"

// Sets default values
ALadder::ALadder()
{
	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("Spline Component"));
	RootComponent = SplineComponent;

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger Volume"));
	TriggerVolume->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ALadder::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALadder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector ALadder::GetGrabLocation(const float InputKey) const
{
	return SplineComponent->GetLocationAtSplineInputKey(InputKey, ESplineCoordinateSpace::World);
}

float ALadder::GetGrabDistance(const FVector StartLocation) const
{
	return SplineComponent->FindInputKeyClosestToWorldLocation(StartLocation);
}

bool ALadder::IsEndOfSpline(const float InputKey) const
{
	const float Distance = SplineComponent->GetDistanceAlongSplineAtSplineInputKey(InputKey);
	return Distance == 0.0f || Distance == SplineComponent->GetSplineLength();
}

bool ALadder::IsTop(const float InputKey) const
{
	const float Distance = SplineComponent->GetDistanceAlongSplineAtSplineInputKey(InputKey);
	return Distance == SplineComponent->GetSplineLength();
}

bool ALadder::IsBottom(const float InputKey) const
{
	const float Distance = SplineComponent->GetDistanceAlongSplineAtSplineInputKey(InputKey);
	return Distance == 0.0f;
}

FRotator ALadder::GetGrabRotation(const FVector StartLocation) const
{
	return GetActorRotation() + FRotator(0.0f, 180.0f, 0.0f);
}

