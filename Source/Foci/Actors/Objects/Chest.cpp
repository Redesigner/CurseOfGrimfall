// Copyright 2023 Stephen Melnick


#include "Chest.h"

#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"

#include "Foci/FociCharacter.h"

AChest::AChest()
{
	ChestMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	RootComponent = ChestMesh;

	PlayerOpenTransform = CreateDefaultSubobject<USceneComponent>(TEXT("PlayerOpenTransform"));
	PlayerOpenTransform->SetupAttachment(RootComponent);
}

void AChest::Interact(AFociCharacter* Source)
{
	Source->SetActorLocationAndRotation(PlayerOpenTransform->GetComponentLocation(), PlayerOpenTransform->GetComponentQuat());
	OnOpen(Source);
	Source->GrantWeapon(ContainedWeapon);
}
