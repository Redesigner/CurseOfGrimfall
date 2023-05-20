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
	UE_LOG(LogTemp, Display, TEXT("Bound viewmodel to widget '%s'"), *GetFName().ToString())
	ViewModel = Model;
}
