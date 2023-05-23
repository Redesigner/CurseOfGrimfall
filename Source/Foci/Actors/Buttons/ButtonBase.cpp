#include "ButtonBase.h"

#include "Foci/Foci.h"
#include "Foci/InteractableInterface.h"
#include "Foci/FociCharacter.h"

AButtonBase::AButtonBase()
{
}

void AButtonBase::Trigger(AActor* Source)
{
	if (bButtonPressed)
	{
		return;
	}
	if (!Target.IsValid())
	{
		UE_LOG(LogButtonSystem, Warning, TEXT("Button '%s' was pressed, but it does not have a target."), *GetFName().ToString())
		return;
	}
	if (!Target->Implements<UInteractableInterface>())
	{
		UE_LOG(LogButtonSystem, Warning, TEXT("Button '%s' was pressed, but target did not implement interactable interface."), *GetFName().ToString())
		return;
	}
	IInteractableInterface::Execute_Interact(Target.Get(), Cast<AFociCharacter>(Source));
	bButtonPressed = true;
	OnTriggered(Source);
}

void AButtonBase::Release(AActor* Source)
{
	if (!bButtonPressed)
	{
		return;
	}
	bButtonPressed = false;
	OnReleased(Source);
}

bool AButtonBase::GetButtonPressed() const
{
	return bButtonPressed;
}

// Called when the game starts or when spawned
void AButtonBase::BeginPlay()
{
	Super::BeginPlay();
}