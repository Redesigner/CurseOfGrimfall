#pragma once

#include "CoreMinimal.h"

#include "Foci/Components/WeaponTool.h"

#include "Hookshot.generated.h"

UENUM(BlueprintType)
enum class EHookshotStatus : uint8
{
	Ready,
	Extending,
	Pulling,
	Retracting
};

UCLASS()
class FOCI_API AHookshot : public AWeaponTool
{
	GENERATED_BODY()

	/*
	* Components!
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* HookshotMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Effect, meta = (AllowPrivateAccess = "true"))
	class UNiagaraComponent* ChainEffect;

public:
	AHookshot(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaTime) override;

	virtual void Fire(class AFociCharacter* Character, FVector DefaultOrigin, FRotator DefaultRotation) override;

	virtual void SetFirstPerson() override;

	virtual void SetThirdPerson() override;

	virtual void DrawWeapon(class AFociCharacter* Character) override;

	void Extend(float DeltaTime);

	void Retract(float DeltaTime);

	// Mark this as a ufunc so we can feed it to our delegate
	UFUNCTION()
	void SetChainLength(float Length);

	UFUNCTION(BlueprintCallable)
	float GetChainLength() const;

	UFUNCTION(BlueprintCallable)
	FVector GetHookLocation() const;

	UFUNCTION(BlueprintCallable)
	EHookshotStatus GetHookshotStatus() const;

	UFUNCTION()
	void OnChainBroken();

private:
	void SuccessfulHit(ACharacter* Character, FVector Location);

	void UpdateVisuals();

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = Hookshot, meta = (AllowPrivateAccess = true))
	EHookshotStatus HookshotStatus;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Hookshot|Chain", meta = (AllowPrivateAccess = true))
	float ChainExtendSpeed = 250.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Hookshot|Chain", meta = (AllowPrivateAccess = true))
	float ChainRetractSpeed = 500.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Hookshot|Chain", meta = (AllowPrivateAccess = true))
	float MaxChainLength = 1000.0f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Hookshot|Chain", meta = (AllowPrivateAccess = true))
	float ChainLength = 0.0f;

	TWeakObjectPtr<AFociCharacter> Owner;

	FVector ChainDirection;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Hookshot|Hook", meta = (AllowPrivateAccess = true))
	FVector HookLocation;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Hookshot|Hook", meta = (AllowPrivateAccess = true))
	float HookRadius = 25.0f;
};
