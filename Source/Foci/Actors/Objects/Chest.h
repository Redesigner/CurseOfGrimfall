// Copyright 2023 Stephen Melnick

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Foci/InteractableInterface.h"
#include "Foci/Components/WeaponTool.h"

#include "Chest.generated.h"

UCLASS()
class FOCI_API AChest : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* ChestMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* PlayerOpenTransform;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Chest, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AWeaponTool> ContainedWeapon;

public:	
	AChest();

	virtual void Interact(class AFociCharacter* Source) override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnOpen(class AFociCharacter* Source);
};
