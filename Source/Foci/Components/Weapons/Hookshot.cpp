#include "Hookshot.h"

#include "NiagaraComponent.h"
#include "Components/SkeletalMeshComponent.h"

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
}

void AHookshot::Fire(AFociCharacter* Character, FVector DefaultOrigin, FRotator DefaultRotation)
{
	check(GetWorld());
	// UE_LOG(LogWeaponSystem, Display, TEXT("Hookshot fired!"))

	const float HookshotDistance = 750.0f;
	const FVector Direction = DefaultRotation.Vector();

	const FVector HookshotEndLocation = DefaultOrigin + Direction * HookshotDistance;
	FHitResult HookshotHitResult;
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(Character);
	CollisionQueryParams.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(HookshotHitResult, DefaultOrigin, HookshotEndLocation, ECC_WorldStatic, CollisionQueryParams);

	if (HookshotHitResult.IsValidBlockingHit())
	{
		SuccessfulHit(Character, HookshotHitResult.Location);
	}
	else
	{
		UE_LOG(LogWeaponSystem, Display, TEXT("Failed to find valid location for the hookshot."))
	}
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

void AHookshot::Draw(class AFociCharacter* Character)
{
	Super::Draw(Character);
}

void AHookshot::SetChainLength(float Length)
{
	ChainLength = Length;
	ChainEffect->SetNiagaraVariableFloat(TEXT("ChainLength"), Length);
}

float AHookshot::GetChainLength() const
{
	return ChainLength;
}

void AHookshot::SuccessfulHit(ACharacter* Character, FVector Location)
{
	// TODO: Restructure the way we access the MarleMovementComponent
	// UE_LOG(LogWeaponSystem, Display, TEXT("Hookshot hit location '%s'"), *Location.ToString())
	AFociCharacter* FociCharacter = Cast<AFociCharacter>(Character);
	FociCharacter->TryDisableFirstPerson();
	UMarleMovementComponent* MarleMovementComponent = Cast<UMarleMovementComponent>(FociCharacter->GetMovementComponent());
	MarleMovementComponent->ActivateTether(Location);
}
