// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_ClearHitboxes.h"

#include "Foci/Components/HitboxController.h"

UAnimNotify_ClearHitboxes::UAnimNotify_ClearHitboxes()
{
#if WITH_EDITOR
	bShouldFireInEditor = false;
#endif
}

#if WITH_EDITOR
bool UAnimNotify_ClearHitboxes::ShouldFireInEditor()
{
	return false;
}
#endif

void UAnimNotify_ClearHitboxes::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp->GetOwner())
	{
		return;
	}
	UActorComponent* HitboxControllerComponent = MeshComp->GetOwner()->GetComponentByClass(UHitboxController::StaticClass());
	if (UHitboxController* HitboxController = Cast<UHitboxController>(HitboxControllerComponent))
	{
		HitboxController->ClearOverlaps();
	}
}
