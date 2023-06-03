// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MarlePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class FOCI_API AMarlePlayerController : public APlayerController
{
	GENERATED_BODY()

private:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Widgets, meta = (AllowPrivateAccess = true))
	TSubclassOf<UUserWidget> DeathWidgetClass;

	UPROPERTY()
	UUserWidget* DeathWidget;

	TWeakObjectPtr<class AFociCharacter> FociCharacterPawn;

protected:
	void OnPossess(APawn* InPawn) override;

public:
	UFUNCTION()
	void OnDeath();

};
