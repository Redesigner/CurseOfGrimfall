#include "Triggerable.h"

ATriggerable::ATriggerable()
{
}

void ATriggerable::Trigger(AActor* Source)
{
	OnTrigger(Source);
}
