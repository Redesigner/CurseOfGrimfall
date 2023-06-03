#include "MarlePlayerController.h"


#include "Blueprint/UserWidget.h" 
#include "Kismet/GameplayStatics.h" 

#include "Foci/FociCharacter.h"
#include "Foci/Components/HealthComponent.h"

void AMarlePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AFociCharacter* FociCharacter = Cast<AFociCharacter>(InPawn);
	if (!FociCharacter)
	{
		return;
	}
	FociCharacter->GetHealthComponent()->OnDeath.AddDynamic(this, &AMarlePlayerController::OnDeath);
	FociCharacterPawn = FociCharacter;
}

void AMarlePlayerController::OnDeath()
{
	if (!DeathWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("No death widget set for MarlePlayerController."))
		return;
	}
	if (FociCharacterPawn->bImmortal)
	{
		return;
	}
	DeathWidget = CreateWidget<UUserWidget>(this, DeathWidgetClass, TEXT("Death Widget"));
	DeathWidget->AddToViewport();
	DeathWidget->SetOwningPlayer(this);
	UGameplayStatics::SetGamePaused(GetWorld(), true);
}
