#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "ViewModelWidget.generated.h"

UCLASS()
class FOCI_API UViewModelWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	virtual void BindViewModel(class UDialogViewModel* Model);

	TWeakObjectPtr<class UDialogViewModel> ViewModel;
	
};
