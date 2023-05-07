// Copyright 2023 Stephen Melnick


#include "DialogTextBlock.h"

#include "Widgets/Text/STextBlock.h"

#include "Foci/Foci.h"
#include "DialogViewModel.h"


void UDialogTextBlock::SetText(FText InText)
{
	if (GetWorld())
	{
		if (GetWorld()->WorldType == EWorldType::Editor)
		{
			Super::SetText(InText);
			return;
		}
	}

	TextString = InText.ToString();
	DisplayText.Empty();
	CurrentTextIndex = 0;
	TextCounter = 0;

	bAnimationPlaying = true;

	MyTextBlock->SetText(FText());
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

		// UE_LOG(LogDialog, Display, TEXT("Dialog animation: '%s'"), *DisplayText);
		
		MyTextBlock->SetText(FText::FromString(DisplayText));
	}
}

void UDialogTextBlock::BindViewModel(UDialogViewModel* DialogViewModel)
{
	ViewModel = DialogViewModel;
}

bool UDialogTextBlock::SkipAnimation()
{
	if (bAnimationPlaying)
	{
		DisplayText = TextString;
		MyTextBlock->SetText(FText::FromString(DisplayText));
		bAnimationPlaying = false;
		return true;
	}
	return false;
}

void UDialogTextBlock::SetDialog(FText Dialog)
{
	UE_LOG(LogDialog, Display, TEXT("Dialog changed to '%s' by callback"), *Dialog.ToString())
	SetText(Dialog);
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