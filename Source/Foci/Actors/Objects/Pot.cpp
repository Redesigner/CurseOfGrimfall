// Copyright 2023 Stephen Melnick
#include "Pot.h"

#include "Foci/Actors/DropTable.h"

APot::APot()
{
	DropTable = CreateDefaultSubobject<UDropTable>(TEXT("Drop Table"));
}

void APot::Break()
{
	DropTable->SpawnDrops(GetActorLocation() + FVector(0.0f, 0.0f, 5.0f), GetActorRotation(), 500.0f);
	OnBreak();
	SetLifeSpan(3.0f);
}

