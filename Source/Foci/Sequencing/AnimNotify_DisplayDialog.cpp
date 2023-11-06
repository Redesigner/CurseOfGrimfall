// Copyright 2023 Stephen Melnick


#include "AnimNotify_DisplayDialog.h"

#include "Foci/FociCharacter.h"

#if WITH_EDITOR
bool UAnimNotify_DisplayDialog::ShouldFireInEditor()
{
	return false;
}
#endif

void UAnimNotify_DisplayDialog::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	AActor* Owner = MeshComp->GetOwner();

	if (AFociCharacter* Character = Cast<AFociCharacter>(Owner))
	{
		Character->SetDialog(Dialog);
	}
}
