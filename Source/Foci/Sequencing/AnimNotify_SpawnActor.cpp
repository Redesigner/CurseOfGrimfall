// Copyright 2023 Stephen Melnick


#include "AnimNotify_SpawnActor.h"

bool UAnimNotify_SpawnActor::ShouldFireInEditor()
{
	return false;
}

void UAnimNotify_SpawnActor::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	UWorld* World = MeshComp->GetWorld();

	if (!World)
	{
		return;
	}

	FVector ActorLocation = MeshComp->GetComponentLocation() + Offset;
	SpawnedActor = World->SpawnActor(Actor, &ActorLocation);
}

void UAnimNotify_SpawnActor::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (SpawnedActor.IsValid())
	{
		SpawnedActor->Destroy();
	}
}


