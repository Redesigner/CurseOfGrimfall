// Copyright 2023 Stephen Melnick


#include "OptionSelector.h"

#include "Foci/Foci.h"

#include "Components/PanelWidget.h"
#include "Components/Button.h"

void UOptionSelector::UpdateButtonCount(int Count)
{
	const int ButtonNum = Buttons.Num();

	if (Count == ButtonNum)
	{
		return;
	}

	// We don't really need to do anything with 0 options, since the widget will be hidden until this number changes
	if (Count == 0)
	{
		return;
	}

	if (Count > ButtonNum)
	{
		UOptionButton* Button;
		
		for (int i = ButtonNum; i < Count; i++)
		{
			Button = NewObject<UOptionButton>(this, ButtonClass);
			Buttons.Add(Button);
			Root->AddChild(Button);
		}

		for (int i = 0; i < Count; i++)
		{
			Button = Buttons[i];


			int LeftIndex = (i - 1 + Count) % Count;
			int RightIndex = (i + 1) % Count;

			Button->SetLeftButton(Buttons[LeftIndex]);
			Button->SetRightButton(Buttons[RightIndex]);
		}
		return;
	}

	if (Count < ButtonNum)
	{
		for (int i = ButtonNum - 1; i >= Count; i--)
		{
			Buttons[i]->RemoveFromParent();
			Buttons.RemoveAt(i);
		}
		return;
	}
}

void UOptionSelector::SetOptions(TArray<FText> Options)
{
	SetVisibility(Options.Num() == 0 ? ESlateVisibility::Hidden : ESlateVisibility::Visible);

	UpdateButtonCount(Options.Num());

	for (int i = 0; i < Options.Num(); i++)
	{
		Buttons[i]->SetText(Options[i]);
	}

	if (Options.Num() >= 1)
	{
		Buttons[0]->Select();
	}
}

uint8 UOptionSelector::GetSelectedOption() const
{
	for (int i = 0; i < Buttons.Num(); i++)
	{
		if (Buttons[i]->HasAnyUserFocus() || Buttons[i]->HasKeyboardFocus())
		{
			UE_LOG(LogDialog, Display, TEXT("Option #%i chosen by user focus."), i)
			return i;
		}
	}
	return 0;
}
