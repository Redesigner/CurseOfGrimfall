// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponTool.generated.h"


UCLASS(config=Game)
class FOCI_API AWeaponTool : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* ThirdPersonRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* FirstPersonRoot;

public:
	AWeaponTool(const FObjectInitializer& ObjectInitializer);

	virtual void Destroyed() override;

	UFUNCTION(BlueprintCallable)
	virtual void SetFirstPerson();

	UFUNCTION(BlueprintCallable)
	virtual void SetThirdPerson();

	void AttachComponentsToSockets(class USkeletalMeshComponent* ThirdPersonMesh, USkeletalMeshComponent* FirstPersonMesh, bool bStartFirstPerson = false, FName SocketName = NAME_None);

	UFUNCTION(BlueprintImplementableEvent)
	void OnFire(class AFociCharacter* Character, FVector DefaultOrigin, FRotator DefaultRotation);

	virtual void Fire(class AFociCharacter* Character, FVector DefaultOrigin, FRotator DefaultRotation);

	virtual void Draw(class AFociCharacter* Character);

	UFUNCTION(BlueprintCallable)
	bool GetWeaponDrawn();

protected:
	virtual void BeginPlay() override;


private:

	bool bFirstPerson = false;

	bool bDrawn = false;
};
