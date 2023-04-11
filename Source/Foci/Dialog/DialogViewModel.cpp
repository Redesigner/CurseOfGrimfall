// Copyright 2023 Stephen Melnick


#include "DialogViewModel.h"

#include "Foci/FociCharacter.h"
#include "Foci/Foci.h"

void UDialogViewModel::SetModel(AFociCharacter* Character)
{
	Model = Character;
}

void UDialogViewModel::SetDialog(FDialogResponse Response)
{
	Dialog = Response;
	OnDialogTextChanged.Broadcast(Dialog.Dialog);
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
	Request.ResponseOption = 0;
	Model->RequestDialogFromLastNpc(Request);
}
