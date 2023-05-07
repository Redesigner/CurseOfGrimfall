// Copyright 2023 Stephen Melnick


#include "DialogViewModel.h"

#include "Foci/FociCharacter.h"
#include "Foci/Foci.h"

#include "Foci/Dialog/DialogWidget.h"

void UDialogViewModel::SetModel(AFociCharacter* Character)
{
	Model = Character;
}

void UDialogViewModel::SetView(UDialogWidget* Widget)
{
	View = Widget;
}

void UDialogViewModel::SetDialog(FDialogResponse Response)
{
	Dialog = Response;
	OnDialogTextChanged.Broadcast(Dialog.Dialog);

	OnDialogOptionsChanged.Broadcast(Dialog.Options);
}

void UDialogViewModel::RequestDialog()
{
	if (!Model.IsValid())
	{
		UE_LOG(LogDialog, Warning, TEXT("DialogViewModel doesn't have a model set. Did you forget to call SetModel()?"))
		return;
	}
	FDialogRequest Request;
	Request.DialogTitle = Dialog.DialogTitle;
	Request.ResponseOption = View->GetSelectedOption();
	Model->RequestDialogFromLastNpc(Request);
}
