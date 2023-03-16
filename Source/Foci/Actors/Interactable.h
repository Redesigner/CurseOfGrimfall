// Copyright 2023 Stephen Melnick

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.generated.h"

UCLASS()
class FOCI_API AInteractable : public AActor
{
	GENERATED_BODY()
	// Should this whole class be an interface instead?
public:	
	// Sets default values for this actor's properties
	AInteractable();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Interact(class ACharacter* Source);

};
