#include "ButtonBase.h"

#include "Foci/Foci.h"
#include "Foci/InteractableInterface.h"
#include "Foci/FociCharacter.h"
#include "Foci/Actors/Buttons/Triggerable.h"

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
	Target->Trigger(Source);
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