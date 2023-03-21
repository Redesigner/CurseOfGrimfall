// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArmorComponent.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum EArmorShape
{
	Sphere,
	Capsule,
	Box
};

USTRUCT(BlueprintType)
struct FOCI_API FArmorInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EArmorShape> Shape;

	UPROPERTY(EditAnywhere, meta = (MakeEditWidget))
		FVector Position;

	UPROPERTY(EditAnywhere)
		FVector Rotation;

	UPROPERTY(EditAnywhere)
		FVector Dimensions;

	UPROPERTY(EditAnywhere)
		FVector Normal;
};

UCLASS()
class FOCI_API UArmorComponent : public USceneComponent
{
	GENERATED_BODY()

	UArmorComponent(const FObjectInitializer& ObjectInitializer);

public:

	virtual void DestroyComponent(bool bPromoteChildren = false) override;

	void SpawnShape(EArmorShape ShapeType, FVector Dimensions);

	bool CanBlockHit(const struct FGameplayAbilitySpec* HitSpec) const;

	void SetNormal(const FVector& InNormal);
	FVector GetNormal() const;

	void SetCollisionProfileName(FName CollisionProfileName);

	void SetDimensions(FVector Dimensions);

	class UShapeComponent* GetShapeComponent();


	TEnumAsByte<EArmorShape> ArmorShape = EArmorShape::Sphere;

	TWeakObjectPtr<class UHitboxController> OwningHitboxController;

	const FGameplayAbilitySpec* OwningAbility;

	FColor Color = FColor::Red;

private:

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere)
	class UArrowComponent* DirectionalArrow;

	UPROPERTY()
	class UShapeComponent* Shape;
#endif

	FVector Normal;
};
