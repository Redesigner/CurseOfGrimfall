#include "PlayerHealthWidget.h"

#include "Components/ProgressBar.h" 
#include "Components/CanvasPanelSlot.h" 

#include "Foci/Dialog/DialogViewModel.h"


void UPlayerHealthWidget::BindViewModel(UDialogViewModel* Model)
{
	Super::BindViewModel(Model);
	if (!ViewModel.IsValid())
	{
		return;
	}
	UpdateHealth(Model->GetHealth(), Model->GetMaxHealth());
	ViewModel->OnHealthUpdated.AddDynamic(this, &UPlayerHealthWidget::UpdateHealth);
}

void UPlayerHealthWidget::UpdateHealth(float NewHealth, float NewMaxHealth)
{
	float Percentage = NewHealth / NewMaxHealth;
	HealthBar->SetPercent(Percentage);

	UCanvasPanelSlot* HealthBarSlot = Cast<UCanvasPanelSlot>(HealthBar->Slot);
	FVector2D HealthBarSize = HealthBarSlot->GetSize();
	HealthBarSize.X = NewMaxHealth * 50.0f;
	HealthBarSlot->SetSize(HealthBarSize);
}
