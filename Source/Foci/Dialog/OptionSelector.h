// Copyright 2023 Stephen Melnick

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "OptionButton.h"

#include "OptionSelector.generated.h"

/**
 * 
 */
UCLASS()
class FOCI_API UOptionSelector : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true", BindWidget))
	class UPanelWidget* Root;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UOptionButton> ButtonClass;

	TArray<UOptionButton*> Buttons;


	void UpdateButtonCount(int Count);

public:
	UFUNCTION()
	void SetOptions(TArray<FText> Options);

	uint8 GetSelectedOption() const;
	
};
