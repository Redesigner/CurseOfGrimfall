#include "Hookshot.h"

#include "NiagaraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"

#include "Foci/Foci.h"
#include "Foci/FociCharacter.h"
#include "Foci/MarleMovementComponent.h"

AHookshot::AHookshot(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	HookshotMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Hookshot Mesh"));
	HookshotMesh->SetupAttachment(ThirdPersonRoot);

	ChainEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Chain Effect"));
	ChainEffect->SetupAttachment(HookshotMesh, TEXT("Hook2"));
	ChainEffect->SetUsingAbsoluteScale(true);
	ChainEffect->SetWorldScale3D(FVector(1.0f));
}

void AHookshot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateVisuals();

	switch (HookshotStatus)
	{
	case(EHookshotStatus::Ready):
		HookLocation = HookshotMesh->GetComponentLocation();
		break;

	case(EHookshotStatus::Extending):
		Extend(DeltaTime);
		break;

	case(EHookshotStatus::Pulling):
	{
		const FVector Distance = HookLocation - GetActorLocation();
		const float Length = Distance.Length();
		SetChainLength(Length);
		break;
	}
	
	case(EHookshotStatus::Retracting):
		Retract(DeltaTime);
		break;
	}
}

void AHookshot::Fire(AFociCharacter* Character, FVector DefaultOrigin, FRotator DefaultRotation)
{
	check(GetWorld());
	
	if (HookshotStatus != EHookshotStatus::Ready)
	{
		return;
	}	
	ChainDirection = DefaultRotation.Vector();
	HookshotStatus = EHookshotStatus::Extending;
	HookLocation = HookshotMesh->GetComponentLocation();
	Owner = Character;
	Character->SetInputEnabled(false);
	Super::Fire(Character, DefaultOrigin, DefaultRotation);
}

void AHookshot::SetFirstPerson()
{
	HookshotMesh->AttachToComponent(FirstPersonRoot, FAttachmentTransformRules::SnapToTargetIncludingScale);
	Super::SetFirstPerson();
}

void AHookshot::SetThirdPerson()
{
	HookshotMesh->AttachToComponent(ThirdPersonRoot, FAttachmentTransformRules::SnapToTargetIncludingScale);
	Super::SetThirdPerson();
}

void AHookshot::DrawWeapon(class AFociCharacter* Character)
{
	UMarleMovementComponent* MovementComponent = Cast<UMarleMovementComponent>(Character->GetMovementComponent());
	MovementComponent->OnTetherBroken.AddDynamic(this, &AHookshot::OnChainBroken);
	Super::DrawWeapon(Character);
}

void AHookshot::Extend(float DeltaTime)
{
	const FVector HookMovement = DeltaTime * ChainExtendSpeed * ChainDirection;
	const FQuat HookQuat;
	FHitResult HookMovementHitResult;
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(this);
	CollisionQueryParams.AddIgnoredActor(Owner.Get());
	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(HookRadius);

	// Sweeping this way allows us to ignore our actors
	if (GetWorld()->SweepSingleByChannel(HookMovementHitResult, HookLocation, HookLocation + HookMovement, HookQuat,
		ECollisionChannel::ECC_WorldStatic, CollisionShape, CollisionQueryParams))
	{
		HookLocation = HookMovementHitResult.Location;
		SuccessfulHit(Owner.Get(), HookMovementHitResult.Location);
		return;
	}
	else
	{
		HookLocation += HookMovement;
	}
	const FVector Distance = HookLocation - GetActorLocation();
	const float Length = Distance.Length();
	SetChainLength(Length);
	if (Length > MaxChainLength)
	{
		HookshotStatus = EHookshotStatus::Retracting;
	}
	// DrawDebugSphere(GetWorld(), HookLocation, HookRadius, 4, FColor::Red, false, DeltaTime * 2.0f);
}

void AHookshot::Retract(float DeltaTime)
{
	const FVector Displacement = HookshotMesh->GetComponentLocation() - HookLocation;
	const FVector Direction = Displacement.GetSafeNormal();

	HookLocation += Direction * DeltaTime * ChainExtendSpeed;

	const FVector NewDisplacement = HookshotMesh->GetComponentLocation() - HookLocation;
	const float Length = NewDisplacement.Length();
	if (Length <= 5.0f)
	{
		SetChainLength(0.0f);
		HookshotStatus = EHookshotStatus::Ready;
		if (Owner.IsValid())
		{
			Owner->SetInputEnabled(true);
		}
	}
	else
	{
		SetChainLength(Length);
	}
}

void AHookshot::SetChainLength(float Length)
{
	UE_LOG(LogWeaponSystem, Display, TEXT("Chain length set to '%f'"), Length)
	ChainLength = Length;
}

float AHookshot::GetChainLength() const
{
	return ChainLength;
}

FVector AHookshot::GetHookLocation() const
{
	return HookLocation;
}

EHookshotStatus AHookshot::GetHookshotStatus() const
{
	return HookshotStatus;
}

void AHookshot::OnChainBroken()
{
	HookshotStatus = EHookshotStatus::Retracting;
}

void AHookshot::SuccessfulHit(ACharacter* Character, FVector Location)
{
	// TODO: Restructure the way we access the MarleMovementComponent
	AFociCharacter* FociCharacter = Cast<AFociCharacter>(Character);
	FociCharacter->TryDisableFirstPerson();
	UMarleMovementComponent* MarleMovementComponent = Cast<UMarleMovementComponent>(FociCharacter->GetMovementComponent());

	HookshotStatus = EHookshotStatus::Pulling;
	MarleMovementComponent->ActivateTether(Location);
	// FociCharacter->SetInputEnabled(false);
}

void AHookshot::UpdateVisuals()
{
	const FVector HookDisplacement = HookshotMesh->GetSocketLocation(TEXT("Chain_2")) - HookshotMesh->GetSocketLocation(TEXT("Hook2"));
	ChainEffect->SetNiagaraVariableFloat(TEXT("ChainLength"), HookDisplacement.Length());
	const FRotator ChainRotation = HookDisplacement.Rotation();
	ChainEffect->SetWorldRotation(ChainRotation);
}
