// Copyright 2023 Stephen Melnick

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "DialogViewModel.h"

#include "DialogWidget.generated.h"

/**
 * 
 */
UCLASS()
class FOCI_API UDialogWidget : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true", BindWidget))
	class UDialogTextBlock* DialogTextBlock;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true", BindWidget))
	class UOptionSelector* DialogOptionSelector;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void Advance();

public:
	UFUNCTION(BlueprintCallable)
	void BindViewModel(UDialogViewModel* DialogViewModel);

	UFUNCTION(BlueprintCallable)
	void DialogChanged(FText Dialog);

	UFUNCTION(BlueprintCallable)
	uint8 GetSelectedOption() const;

private:
	TWeakObjectPtr<UDialogViewModel> DialogViewModel;
};
