#include "ViewModelWidget.h"

#include "Foci/Dialog/DialogViewModel.h"
#include "Foci/FociCharacter.h"


void UViewModelWidget::NativeConstruct()
{
	AFociCharacter* FociCharacter = Cast<AFociCharacter>(GetOwningPlayer()->GetCharacter());
	BindViewModel(FociCharacter->DialogViewModel);
}

void UViewModelWidget::BindViewModel(UDialogViewModel* Model)
{
	ViewModel = Model;
}
