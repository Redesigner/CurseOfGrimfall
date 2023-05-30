#pragma once

#include "CoreMinimal.h"

#include "Foci/Components/WeaponTool.h"

#include "Hookshot.generated.h"

UCLASS()
class FOCI_API AHookshot : public AWeaponTool
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* HookshotMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Effect, meta = (AllowPrivateAccess = "true"))
	class UNiagaraComponent* ChainEffect;

public:
	AHookshot(const FObjectInitializer& ObjectInitializer);

	virtual void Fire(class AFociCharacter* Character, FVector DefaultOrigin, FRotator DefaultRotation) override;

	virtual void SetFirstPerson() override;

	virtual void SetThirdPerson() override;

	virtual void Draw(class AFociCharacter* Character) override;

	// Mark this as a ufunc so we can feed it to our delegate
	UFUNCTION()
	void SetChainLength(float Length);

	float GetChainLength() const;

private:
	void SuccessfulHit(ACharacter* Character, FVector Location);

	float ChainLength = 0.0f;
};
