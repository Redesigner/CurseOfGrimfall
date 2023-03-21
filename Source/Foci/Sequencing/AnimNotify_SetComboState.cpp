// Fill out your copyright notice in the Description page of Project Settings.
#include "AnimNotify_SetComboState.h"

#include "Foci/FociCharacter.h"

UAnimNotify_SetComboState::UAnimNotify_SetComboState()
{
	bShouldFireInEditor = false;
}

bool UAnimNotify_SetComboState::ShouldFireInEditor()
{
	return false;
}

void UAnimNotify_SetComboState::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
}
