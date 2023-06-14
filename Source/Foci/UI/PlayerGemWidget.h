#pragma once

#include "CoreMinimal.h"
#include "Foci/UI/ViewModelWidget.h"
#include "PlayerGemWidget.generated.h"

UCLASS()
class FOCI_API UPlayerGemWidget : public UViewModelWidget
{
	GENERATED_BODY()

	virtual void BindViewModel(class UDialogViewModel* Model) override;

	UFUNCTION()
	void UpdateGemCount(uint8 GemCount);

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true", BindWidget))
	class UTextBlock* GemCountDisplay;
};
