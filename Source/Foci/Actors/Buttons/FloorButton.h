#pragma once

#include "CoreMinimal.h"

#include "Foci/Actors/Buttons/ButtonBase.h"

#include "FloorButton.generated.h"

UCLASS()
class FOCI_API AFloorButton : public AButtonBase
{
	GENERATED_BODY()

private:
	AFloorButton(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Components, meta = (AllowPrivateAccess = true))
	class USkeletalMeshComponent* ButtonMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Components, meta = (AllowPrivateAccess = true))
	class UBoxComponent* Box;


public:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	class UBoxComponent* GetSurface() const;
};
