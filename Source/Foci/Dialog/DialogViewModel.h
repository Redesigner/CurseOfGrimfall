// Copyright 2023 Stephen Melnick

#pragma once

#include "CoreMinimal.h"
// #include "MVVMViewModelBase.h"
#include "DialogRequest.h"
#include "DialogResponse.h"

#include "DialogViewModel.generated.h"

/**
 * 
 */
 // Should be renamed to a generic viewmodel for player?
UCLASS()
class FOCI_API UDialogViewModel : public UObject
{
	GENERATED_BODY()

public:
	void SetModel(class AFociCharacter* Character);

	void SetDialogView(class UDialogWidget* Widget);

	void SetDialog(FDialogResponse Response);

	void RequestDialog();

	UFUNCTION()
	void HealthChanged(float NewHealth, float HealthPercentage);

	UFUNCTION()
	void MaxHealthChanged(float NewMaxHealth);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogOptionsChanged, TArray<FText>, Options);

	UPROPERTY(BlueprintAssignable)
	FOnDialogOptionsChanged OnDialogOptionsChanged;


	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogTextChanged, FText, NewText);

	UPROPERTY(BlueprintAssignable)
	FOnDialogTextChanged OnDialogTextChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthUpdated, float, Health, float, MaxHealth);

	UPROPERTY(BlueprintAssignable)
	FOnHealthUpdated OnHealthUpdated;

private:
	FDialogResponse Dialog;

	TWeakObjectPtr<class AFociCharacter> Model;

	TWeakObjectPtr<class UDialogWidget> DialogView;

	float Health;

	float MaxHealth;
};
