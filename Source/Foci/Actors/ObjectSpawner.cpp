#include "ObjectSpawner.h"

void AObjectSpawner::BeginPlay()
{
	Super::BeginPlay();

	Spawn();
}


void AObjectSpawner::Spawn()
{
	if (!ObjectToSpawn)
	{
		return;
	}
	SpawnedObject = GetWorld()->SpawnActor<AActor>(ObjectToSpawn, GetActorLocation(), GetActorRotation());
	SpawnedObject->OnDestroyed.AddDynamic(this, &AObjectSpawner::OnObjectDestroyed);
}

void AObjectSpawner::OnObjectDestroyed(AActor* DestroyedActor)
{
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &AObjectSpawner::Spawn);
	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, TimerDelegate, RespawnTime, false);
}
