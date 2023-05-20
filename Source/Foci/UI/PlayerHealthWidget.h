#pragma once

#include "CoreMinimal.h"

#include "Foci/UI/ViewModelWidget.h"

#include "PlayerHealthWidget.generated.h"

UCLASS()
class FOCI_API UPlayerHealthWidget : public UViewModelWidget
{
	GENERATED_BODY()

	virtual void BindViewModel(class UDialogViewModel* Model) override;

	UFUNCTION()
	void UpdateHealth(float NewHealth, float NewMaxHealth);

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true", BindWidget))
	class UProgressBar* HealthBar;
};
