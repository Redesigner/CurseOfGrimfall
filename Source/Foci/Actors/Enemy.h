#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Enemy.generated.h"

UCLASS()
class FOCI_API AEnemy : public APawn
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class UCapsuleComponent* Capsule;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class UEnemyMovementComponent* MovementComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Drops, meta = (AllowPrivateAccess = "true"))
	class UDropTable* DropTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Drops, meta = (AllowPrivateAccess = "true"))
	class UHealthComponent* HealthComponent;

public:
	AEnemy(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable)
	class UHealthComponent* GetHealthComponent() const;

	UFUNCTION(BlueprintCallable)
	class UCapsuleComponent* GetCapsule() const;

	UFUNCTION(BlueprintCallable)
	void MoveForward();

	UFUNCTION()
	void OnDeath();
};
