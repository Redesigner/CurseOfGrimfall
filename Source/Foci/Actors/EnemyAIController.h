#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

UCLASS()
class FOCI_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

private:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Targeting, meta = (AllowPrivateAccess = true))
	TWeakObjectPtr<APawn> CurrentTarget;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Targeting|Walking", meta = (AllowPrivateAccess = true))
	float MaxDistance = 50.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Targeting|Walking", meta = (AllowPrivateAccess = true))
	float MinDistance = 25.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Targeting|Sight", meta = (AllowPrivateAccess = true))
	float ConeViewingAngle = 60.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Targeting|Sight", meta = (AllowPrivateAccess = true))
	float ViewDistance = 1000.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Behavior, meta = (AllowPrivateAccess = true))
	bool bStrafing = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Behavior|Attacking", meta = (AllowPrivateAccess = true))
	float AttackCooldown = 3.0f;

	FTimerHandle AttackTimerHandle;
	

protected:
	virtual void OnPossess(APawn* InPawn) override;

	void SetTarget(APawn* Target);

	virtual FVector GetDestination() const;

	bool CanSeeTarget(APawn* Target) const;

	void SetStrafing(bool Value);

	bool IsCloseToTarget(APawn* Target, float Distance) const;

	void TryExecuteAttack();


	TWeakObjectPtr<class AEnemy> EnemyPawn;

public:
	virtual void Tick(float DeltaTime) override;

	// Try to set the target externally, typically when the enemy is hit
	virtual void TrySetTarget(APawn* Target);
};
