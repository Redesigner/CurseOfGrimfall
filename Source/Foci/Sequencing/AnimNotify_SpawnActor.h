// Copyright 2023 Stephen Melnick

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotify_SpawnActor.generated.h"

/**
 * 
 */
UCLASS()
class FOCI_API UAnimNotify_SpawnActor : public UAnimNotifyState
{
	GENERATED_BODY()
	
	virtual bool ShouldFireInEditor() override;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<AActor> Actor;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector Offset;

	TWeakObjectPtr<AActor> SpawnedActor;
};
