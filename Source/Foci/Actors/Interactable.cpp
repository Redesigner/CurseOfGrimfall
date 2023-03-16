// Copyright 2023 Stephen Melnick


#include "Interactable.h"

// Sets default values
AInteractable::AInteractable()
{
}

// Called when the game starts or when spawned
void AInteractable::BeginPlay()
{
	Super::BeginPlay();
	
}


void AInteractable::Interact_Implementation(ACharacter* Source)
{
	UE_LOG(LogTemp, Display, TEXT("Player interacted with object"))
}

