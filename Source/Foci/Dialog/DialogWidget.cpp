// Copyright 2023 Stephen Melnick


#include "DialogWidget.h"

#include "DialogTextBlock.h"
#include "OptionSelector.h"

void UDialogWidget::NativeConstruct()
{
	Super::NativeConstruct();

}

void UDialogWidget::BindViewModel(UDialogViewModel* Model)
{
	Super::BindViewModel(Model);

	DialogTextBlock->BindViewModel(Model);
	ViewModel->SetDialogView(this);
	ViewModel->OnDialogTextChanged.AddDynamic(DialogTextBlock, &UDialogTextBlock::SetDialog);
	ViewModel->OnDialogTextChanged.AddDynamic(this, &UDialogWidget::DialogChanged);

	ViewModel->OnDialogOptionsChanged.AddDynamic(DialogOptionSelector, &UOptionSelector::SetOptions);
}

void UDialogWidget::DialogChanged(FText Dialog)
{
	if (Dialog.ToString().Len() == 0)
	{
		SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		SetVisibility(ESlateVisibility::Visible);
	}
}

uint8 UDialogWidget::GetSelectedOption() const
{
	return DialogOptionSelector->GetSelectedOption();
}

void UDialogWidget::Advance()
{
	if (!DialogTextBlock->SkipAnimation())
	{
		ViewModel->RequestDialog();
	}
}
