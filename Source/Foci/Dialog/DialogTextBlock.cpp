// Copyright 2023 Stephen Melnick


#include "DialogTextBlock.h"

#include "Widgets/Text/STextBlock.h"

#include "Foci/Foci.h"
#include "DialogViewModel.h"

TAttribute<FText> UDialogTextBlock::GetDisplayText()
{
	if (!GetWorld())
	{
		return FText::FromString("Dialog Text Block");
	}
	if (GetWorld()->WorldType == EWorldType::Editor)
	{
		return FText::FromString("Dialog Text Block");
	}
	return FText::FromString(DisplayText);
}

void UDialogTextBlock::SetText(FText InText)
{
	Super::SetText(InText);

	TextString = InText.ToString();
	DisplayText.Empty();
	CurrentTextIndex = 0;
	TextCounter = 0;

	bAnimationPlaying = true;

	// This will force an update of all properties, invoking GetDisplayText().
	// AFAIK, this is the best way to update the text without accessing slate directly
	Super::SynchronizeProperties();
}

void UDialogTextBlock::Tick(float DeltaTime)
{
	if (!bAnimationPlaying)
	{
		return;
	}
	if (CurrentTextIndex >= TextString.Len())
	{
		bAnimationPlaying = false;
		TextCounter = 0;
		return;
	}
	TextCounter += CharactersPerSecond * DeltaTime;
	if (TextCounter >= 1)
	{
		TextCounter--;
		CurrentTextIndex++;
		DisplayText += TextString[CurrentTextIndex - 1];
		Super::SynchronizeProperties();
	}
}

void UDialogTextBlock::SetViewModel(UDialogViewModel* DialogViewModel)
{
	ViewModel = DialogViewModel;
}

void UDialogTextBlock::Advance()
{
	if (bAnimationPlaying)
	{
		DisplayText = TextString;
		Super::SynchronizeProperties();
		bAnimationPlaying = false;
		return;
	}
	if (!ViewModel.IsValid())
	{
		UE_LOG(LogDialog, Warning, TEXT("DialogTextBlock does not have a valid viewmodel."))
		return;
	}
	// @TODO: respond appropriately to options. For now, viewmodel handles actually generating the request
	// Should this be changed?
	ViewModel->RequestDialog();
}

/////////////////////////////////////////////////////////////////////////////
// Tickable Interface functions
ETickableTickType UDialogTextBlock::GetTickableTickType() const
{
	return ETickableTickType::Always;
}

TStatId UDialogTextBlock::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UDialogTextBlock, STATGROUP_Tickables);
}

bool UDialogTextBlock::IsTickableWhenPaused() const
{
	return true;
}

bool UDialogTextBlock::IsTickableInEditor() const
{
	return false;
}
////////////////////////////////////////////////////////////////////////////