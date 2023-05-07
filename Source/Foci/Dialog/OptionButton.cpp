// Copyright 2023 Stephen Melnick


#include "OptionButton.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UOptionButton::NativeConstruct()
{
	Super::NativeConstruct();

	bIsFocusable = true;
}

UOptionButton* UOptionButton::GetLeftButton() const
{
	return LeftButton.Get();
}

UOptionButton* UOptionButton::GetRightButton() const
{
	return RightButton.Get();
}

void UOptionButton::SetText(FText Text)
{
	TextBlock->SetText(Text);
}

void UOptionButton::Select()
{
	SetFocus();
}

void UOptionButton::SetLeftButton(UOptionButton* Button)
{
	LeftButton = Button;
	SetNavigationRuleExplicit(EUINavigation::Left, Button);
}

void UOptionButton::SetRightButton(UOptionButton* Button)
{
	RightButton = Button;
	SetNavigationRuleExplicit(EUINavigation::Right, Button);
}
