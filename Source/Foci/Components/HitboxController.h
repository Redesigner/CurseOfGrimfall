// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Foci\Components\ArmorComponent.h"
#include "Components/SphereComponent.h"

#include "HitboxController.generated.h"

class UArmorComponent;
UCLASS()
class UHitboxController : public USceneComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	
	// ===== Hitbox manipulation functions =====
	TWeakObjectPtr<UArmorComponent> GetHitboxByName(FName Name);

	TWeakObjectPtr<UArmorComponent> SpawnHitbox(FName Name, FVector HitboxRelativeLocation, FVector Rotation, FVector Direction, FVector Dimensions, EArmorShape Shape);

	void RemoveHitboxByName(FName Name);
	// ===== End hitbox manipulation =====



	// ===== Armor manipulation functions =====
	TWeakObjectPtr<UArmorComponent> GetArmorByName(FName Name);

	TWeakObjectPtr<UArmorComponent> SpawnArmor(FName Name, FVector ArmorRelativeLocation, FVector Rotation, FVector Normal, FVector Dimensions, EArmorShape Shape);

	void RemoveArmorByName(FName Name);
	// ===== End armor manipulation =====


	UFUNCTION()
	void AccumulateOverlaps(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	
	UFUNCTION()
	void ClearOverlaps();

	void SetCurrentGameplayAbilitySpec(const FGameplayAbilitySpec* AbilitySpec);


	DECLARE_DELEGATE_OneParam(FOnHitDetected, const FHitResult&);
	FOnHitDetected HitDetectedDelegate;

	DECLARE_DELEGATE_OneParam(FOnHitInterrupted, const FHitResult&);
	FOnHitInterrupted HitInterruptedDelegate;

	DECLARE_DELEGATE_OneParam(FOnHitBlocked, const FHitResult&);
	FOnHitBlocked HitBlockedDelegate;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Collision)
	FCollisionProfileName HitboxCollisionProfile;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Collision)
	FCollisionProfileName ArmorCollisionProfile;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Collision)
	TEnumAsByte<ECollisionChannel> BlockCollisionChannel;

	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = Collision, Transient)
	TArray<FHitResult> QueuedHits;

private:
	/// Check to see if there is an armor component inbetween:
	///		1) the part of the target that was hit
	///		2) our base position
	/// Additionally, check if the armor is 'strong' enough to block the hit
	bool GetIsHitBlocked(const UArmorComponent* Hitbox, const FVector& HitLocation) const;

	// It's got a little more state than I'd prefer, but ¯\_()_/¯
	const FGameplayAbilitySpec* CurrentAbilitySpec;
};
