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

	void UpdateHealthBarMaximum(float NewMaxHealth);

	void SetImageDisplays(float Health);


	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true", BindWidget))
	class UHorizontalBox* HorizontalBox;

	float MaxHealth = 1.0f;

	float CurrentHealth = 1.0f;

	TArray<class UImage*> HeartDisplays;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Textures)
	UTexture2D* EmptyTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Textures)
	UTexture2D* HalfTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Textures)
	UTexture2D* FullTexture;
};
