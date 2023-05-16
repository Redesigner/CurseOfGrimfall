#include "EnemyAIController.h"

#include "Blueprint/AIBlueprintHelperLibrary.h" 

void AEnemyAIController::SetNewMoveDestination(FVector Destination)
{
	UNavigationSystemBase* NavigationSystem = GetWorld()->GetNavigationSystem();
	UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, Destination);
}
