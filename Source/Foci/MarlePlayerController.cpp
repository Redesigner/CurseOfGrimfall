#include "MarlePlayerController.h"


#include "Blueprint/UserWidget.h" 
#include "Kismet/GameplayStatics.h" 

#include "Foci/FociCharacter.h"
#include "Foci/FociGameMode.h"
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


	if (AFociGameMode* GameMode = Cast<AFociGameMode>(UGameplayStatics::GetGameMode(GetWorld())) )
	{
		GameMode->bWorldActive = false;
	}
	FTimerHandle DeathTimerHandle;
	FTimerDelegate DeathDelegate;
	DeathDelegate.BindLambda([this]() {
			DeathWidget = CreateWidget<UUserWidget>(this, DeathWidgetClass, TEXT("Death Widget"));
			DeathWidget->AddToViewport();
			DeathWidget->SetOwningPlayer(this);
			UGameplayStatics::SetGamePaused(GetWorld(), true);
			bShowMouseCursor = true;
		});
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.SetTimer(DeathTimerHandle, DeathDelegate, 1.0f, false);
}
