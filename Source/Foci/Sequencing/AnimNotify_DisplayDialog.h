// Copyright 2023 Stephen Melnick

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Foci/Dialog/DialogResponse.h"

#include "AnimNotify_DisplayDialog.generated.h"

/**
 * 
 */
UCLASS()
class FOCI_API UAnimNotify_DisplayDialog : public UAnimNotify
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual bool ShouldFireInEditor() override;
#endif
	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Dialog)
	FDialogResponse Dialog;
};
