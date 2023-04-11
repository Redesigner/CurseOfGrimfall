// Copyright 2023 Stephen Melnick

#pragma once

#include "CoreMinimal.h"

#include "DialogResponse.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FOCI_API FDialogResponse
{
	GENERATED_BODY()

	bool IsEmpty() const;


	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FText DialogTitle;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FText Dialog;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FText> Options;
};
