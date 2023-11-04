#include "PlayerHealthWidget.h"

#include "Components/ProgressBar.h" 
#include "Components/CanvasPanelSlot.h" 
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"

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
	/* float Percentage = NewHealth / NewMaxHealth;
	HealthBar->SetPercent(Percentage);

	UCanvasPanelSlot* HealthBarSlot = Cast<UCanvasPanelSlot>(HealthBar->Slot);
	FVector2D HealthBarSize = HealthBarSlot->GetSize();
	HealthBarSize.X = NewMaxHealth * 50.0f;
	HealthBarSlot->SetSize(HealthBarSize); */

	UpdateHealthBarMaximum(NewMaxHealth);
	SetImageDisplays(NewHealth);
}

void UPlayerHealthWidget::UpdateHealthBarMaximum(float NewMaxHealth)
{
	if (NewMaxHealth == MaxHealth)
	{
		// No update necessary
		return;
	}
	MaxHealth = NewMaxHealth;

	for (UImage* Image : HeartDisplays)
	{
		if (Image)
		{
			HorizontalBox->RemoveChild(Image);
		}
	}
	HeartDisplays.Empty();

	int NumHeartDisplays = FMath::CeilToInt(MaxHealth);
	for (int i = 0; i < NumHeartDisplays; i++)
	{
		UImage* NewHeartDisplay = NewObject<UImage>();
		HorizontalBox->AddChild(NewHeartDisplay);
		HeartDisplays.Add(NewHeartDisplay);
		NewHeartDisplay->SetVisibility(ESlateVisibility::Visible);
		NewHeartDisplay->SetBrushSize(FVector2D(64.0f, 50.0f)); // Don't hardcode this
		UHorizontalBoxSlot* HeartSlot = Cast<UHorizontalBoxSlot>(NewHeartDisplay->Slot);
		HeartSlot->SetPadding(FMargin(0.0f, 0.0f, 5.0f, 0.0f));
		
	}
}

void UPlayerHealthWidget::SetImageDisplays(float Health)
{
	if (Health == CurrentHealth)
	{
		return;
	}
	CurrentHealth = FMath::Max(0.0f, Health);

	if (CurrentHealth == 0.0f)
	{
		for (UImage* HeartDisplay : HeartDisplays)
		{
			HeartDisplay->SetBrushFromTexture(EmptyTexture);
		}
		return;
	}

	const int LastFullIndex = FMath::Floor(Health) - 1;

	for (int i = 0; i <= LastFullIndex; i++)
	{
		if (i >= HeartDisplays.Num() || !HeartDisplays[i])
		{
			return;
		}
		HeartDisplays[i]->SetBrushFromTexture(FullTexture);
	}
	int EmptyIndex = LastFullIndex + 1;

	// Set the half heart if necessary
	if (Health - EmptyIndex > 0.0f)
	{
		if (EmptyIndex >= HeartDisplays.Num() || !HeartDisplays[EmptyIndex])
		{
			return;
		}
		HeartDisplays[EmptyIndex]->SetBrushFromTexture(HalfTexture);
		EmptyIndex++;
	}
	for (int i = EmptyIndex; i < HeartDisplays.Num(); i++)
	{
		if (HeartDisplays[i])
		{
			HeartDisplays[i]->SetBrushFromTexture(EmptyTexture);
		}
	}
}
