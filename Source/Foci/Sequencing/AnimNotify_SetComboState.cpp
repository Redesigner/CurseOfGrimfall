// Fill out your copyright notice in the Description page of Project Settings.
#include "AnimNotify_SetComboState.h"

#include "Foci/FociCharacter.h"

UAnimNotify_SetComboState::UAnimNotify_SetComboState()
{
#if WITH_EDITOR
	bShouldFireInEditor = false;
#endif
}

#if WITH_EDITOR
bool UAnimNotify_SetComboState::ShouldFireInEditor()
{
	return false;
}
#endif

void UAnimNotify_SetComboState::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
}
