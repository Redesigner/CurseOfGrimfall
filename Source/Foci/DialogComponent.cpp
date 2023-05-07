#include "DialogComponent.h"

#include "Foci.h"
#include "Foci/Dialog/DialogResponse.h"
#include "Foci/Dialog/DialogTableEntry.h"
#include "Foci/FociCharacter.h"

void UDialogComponent::RequestDialog(AFociCharacter* RequestSource, FDialogRequest Request)
{
	RequestSource->SetDialog(GetDialogResponse(Request));
}

FDialogResponse UDialogComponent::GetDialogResponse(FDialogRequest& Request) const
{
	if (DialogTable.IsNull())
	{
		UE_LOG(LogDialog, Warning, TEXT("Dialog requested, but no Dialog Table has been set. '%s'"), *GetPathNameSafe(this))
		return FDialogResponse();
	}

	if (DialogTable.IsPending())
	{
		UE_LOG(LogDialog, Warning, TEXT("Dialog requested, but the dialog table at '%s' is not loaded. '%s'"), *DialogTable.ToSoftObjectPath().ToString(), *GetPathNameSafe(this))
			DialogTable.LoadSynchronous();
	}

	TArray<FDialogTableEntry*> TableEntries;
	DialogTable->GetAllRows(TEXT(""), TableEntries);
	FText PreviousDialogInstanceName = Request.DialogTitle.IsEmpty() ? FText::FromString(TEXT("Default")) : Request.DialogTitle;

	for (FDialogTableEntry* TableEntry : TableEntries)
	{
		if (TableEntry->PreviousDialogInstance.EqualTo(Request.DialogTitle) && TableEntry->OptionChosen == Request.ResponseOption)
		{
			return TableEntry->DialogResponse;
		}
	}
	UE_LOG(LogDialog, Display, TEXT("No dialog response found matching name: '%s' and option: '%i'"), *Request.DialogTitle.ToString(), Request.ResponseOption)
	return FDialogResponse();
}
