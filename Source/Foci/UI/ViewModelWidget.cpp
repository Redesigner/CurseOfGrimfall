#include "ViewModelWidget.h"

#include "Foci/Dialog/DialogViewModel.h"
#include "Foci/FociCharacter.h"


void UViewModelWidget::NativeConstruct()
{
	AFociCharacter* FociCharacter = Cast<AFociCharacter>(GetOwningPlayer()->GetCharacter());
	if (!FociCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("Unable to get character for viewmodel."));
		return;
	}
	if (!FociCharacter->GetViewModel())
	{
		UE_LOG(LogTemp, Error, TEXT("Unable to viewmodel from character."));
		return;
	}
	BindViewModel(FociCharacter->GetViewModel());
}

void UViewModelWidget::BindViewModel(UDialogViewModel* Model)
{
	UE_LOG(LogTemp, Display, TEXT("Bound viewmodel to widget '%s'"), *GetFName().ToString())
	ViewModel = Model;
}
