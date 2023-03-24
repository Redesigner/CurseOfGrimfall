// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponTool.h"

#include "Components/SceneComponent.h"

AWeaponTool::AWeaponTool(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));

	ThirdPersonRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Third Person Root"));
	ThirdPersonRoot->SetupAttachment(RootComponent);

	FirstPersonRoot = CreateDefaultSubobject<USceneComponent>(TEXT("First Person Root"));
	FirstPersonRoot->SetupAttachment(RootComponent);
}

void AWeaponTool::Destroyed()
{
	if (FirstPersonRoot)
	{
		FirstPersonRoot->DestroyComponent();
	}
	if (ThirdPersonRoot)
	{
		ThirdPersonRoot->DestroyComponent();
	}
	Super::Destroyed();
}

void AWeaponTool::SetFirstPerson()
{
	bFirstPerson = true;
	FirstPersonRoot->SetVisibility(true, true);
	ThirdPersonRoot->SetVisibility(false, true);
}

void AWeaponTool::SetThirdPerson()
{
	bFirstPerson = false;
	ThirdPersonRoot->SetVisibility(true, true);
	FirstPersonRoot->SetVisibility(false, true);
}


void AWeaponTool::AttachComponentsToSockets(USkeletalMeshComponent* ThirdPersonMesh, USkeletalMeshComponent* FirstPersonMesh, bool bStartFirstPerson, FName SocketName)
{
	ThirdPersonRoot->AttachToComponent(ThirdPersonMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
	FirstPersonRoot->AttachToComponent(FirstPersonMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, NAME_None);
	if (bStartFirstPerson)
	{
		SetFirstPerson();
	}
	else
	{
		SetThirdPerson();
	}
}

void AWeaponTool::Fire(ACharacter* Character, FVector DefaultOrigin, FRotator DefaultRotation)
{
	OnFire(Character, DefaultOrigin, DefaultRotation);
	bDrawn = false;
}

void AWeaponTool::Draw()
{
	bDrawn = true;
}

bool AWeaponTool::GetWeaponDrawn()
{
	return bDrawn;
}

void AWeaponTool::BeginPlay()
{
	Super::BeginPlay();
}

