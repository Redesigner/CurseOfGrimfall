// Copyright 2023 Stephen Melnick

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OptionButton.generated.h"

/**
 * 
 */
UCLASS()
class FOCI_API UOptionButton : public UUserWidget
{
	GENERATED_BODY()

	// UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true", BindWidget))
	// class UButton* Button;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true", BindWidget))
	class UTextBlock* TextBlock;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	UOptionButton* GetLeftButton() const;

	UFUNCTION(BlueprintCallable)
	UOptionButton* GetRightButton() const;


public:
	void SetText(FText Text);

	void Select();

	void SetLeftButton(UOptionButton* Button);

	void SetRightButton(UOptionButton* Button);

	// class UButton* GetButton();

private:
	TWeakObjectPtr<UOptionButton> LeftButton;

	TWeakObjectPtr<UOptionButton> RightButton;
};
