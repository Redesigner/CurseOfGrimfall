// Copyright 2023 Stephen Melnick


#include "DialogViewModel.h"

#include "Foci/FociCharacter.h"
#include "Foci/Foci.h"
#include "Foci/Dialog/DialogWidget.h"
#include "Foci/Components/HealthComponent.h"

void UDialogViewModel::SetModel(AFociCharacter* Character)
{
	Model = Character;
	UE_LOG(LogTemp, Display, TEXT("Manually updating player health."))
	OnHealthUpdated.Broadcast(Character->GetHealthComponent()->GetCurrentHealth(), Character->GetHealthComponent()->GetMaxHealth());
}

void UDialogViewModel::SetDialogView(UDialogWidget* Widget)
{
	DialogView = Widget;
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
	Request.ResponseOption = DialogView->GetSelectedOption();
	Model->RequestDialogFromLastNpc(Request);
}

void UDialogViewModel::HealthChanged(float NewHealth, float HealthPercentage)
{
	Health = NewHealth;
	OnHealthUpdated.Broadcast(NewHealth, MaxHealth);
}

void UDialogViewModel::MaxHealthChanged(float NewMaxHealth)
{
	MaxHealth = NewMaxHealth;
	OnHealthUpdated.Broadcast(Health, NewMaxHealth);
}

float UDialogViewModel::GetHealth() const
{
	return Health;
}

float UDialogViewModel::GetMaxHealth() const
{
	return MaxHealth;
}
