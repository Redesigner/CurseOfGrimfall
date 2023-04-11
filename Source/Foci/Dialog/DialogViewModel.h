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
UCLASS()
class FOCI_API UDialogViewModel : public UObject
{
	GENERATED_BODY()

public:
	void SetModel(class AFociCharacter* Character);

	void SetDialog(FDialogResponse Response);

	void RequestDialog();


	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogTextChanged, FText, NewText);

	UPROPERTY(BlueprintAssignable)
	FOnDialogTextChanged OnDialogTextChanged;

private:
	FDialogResponse Dialog;

	TWeakObjectPtr<class AFociCharacter> Model;
};
