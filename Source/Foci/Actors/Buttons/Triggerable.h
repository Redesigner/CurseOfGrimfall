#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Triggerable.generated.h"

UCLASS()
class FOCI_API ATriggerable : public AActor
{
	GENERATED_BODY()

public:
	ATriggerable();

	virtual void Trigger(AActor* Source);

	UFUNCTION(BlueprintImplementableEvent)
	void OnTrigger(AActor* Source);
};
