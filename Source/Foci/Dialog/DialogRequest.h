// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "DialogRequest.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FOCI_API FDialogRequest
{
	GENERATED_BODY()


	UPROPERTY(BlueprintReadWrite)
	FText DialogTitle;

	UPROPERTY(BlueprintReadWrite)
	uint8 ResponseOption = 0;
};
