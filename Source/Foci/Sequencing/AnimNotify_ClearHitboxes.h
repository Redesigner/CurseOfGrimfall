// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_ClearHitboxes.generated.h"

/**
 * This notification needs to be branching, if used in multiplayer. I can potentially implement accumulating states, but for now, it can skip
 */
UCLASS()
class FOCI_API UAnimNotify_ClearHitboxes : public UAnimNotify
{
	GENERATED_BODY()

	UAnimNotify_ClearHitboxes();

public:

	virtual bool ShouldFireInEditor() override;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
