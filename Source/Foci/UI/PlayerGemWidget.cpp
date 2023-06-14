#include "PlayerGemWidget.h"

#include "Components/TextBlock.h"

#include "Foci/Dialog/DialogViewModel.h"

void UPlayerGemWidget::BindViewModel(UDialogViewModel* Model)
{
	Super::BindViewModel(Model);

	ViewModel->OnGemsUpdated.AddDynamic(this, &UPlayerGemWidget::UpdateGemCount);
	UpdateGemCount(0);
}

void UPlayerGemWidget::UpdateGemCount(uint8 GemCount)
{
	FString DisplayString = FString::Printf(TEXT("%u"), GemCount);
	FText DisplayText = FText::FromString(DisplayString);
	GemCountDisplay->SetText(DisplayText);
}
