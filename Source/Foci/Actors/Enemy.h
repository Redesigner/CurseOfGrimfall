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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class UHitboxController* HitboxController;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Drops, meta = (AllowPrivateAccess = "true"))
	class UDropTable* DropTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Drops, meta = (AllowPrivateAccess = "true"))
	class UHealthComponent* HealthComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attacks, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attacks, meta = (AllowPrivateAccess = "true"))
	float AttackDamage;

public:
	AEnemy(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable)
	class UHealthComponent* GetHealthComponent() const;

	UFUNCTION(BlueprintCallable)
	class UCapsuleComponent* GetCapsule() const;

	UFUNCTION(BlueprintCallable)
	class UEnemyMovementComponent* GetEnemyMovementComponent() const;

	UFUNCTION(BlueprintCallable)
	const class USkeletalMeshComponent* GetSkeletalMeshComponent() const;

	UFUNCTION(BlueprintCallable)
	void MoveForward();

	UFUNCTION()
	void OnDeath();

	UFUNCTION(BlueprintImplementableEvent)
	void Death();

	void Attack();

	UFUNCTION()
	void HitTarget_Internal(const FHitResult& HitResult);

	UFUNCTION()
	void OnHit(APawn* Attacker);

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;
};
