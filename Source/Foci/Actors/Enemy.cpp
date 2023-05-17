#include "Enemy.h"

#include "Foci/Actors/DropTable.h"
#include "Foci/Components/HealthComponent.h"
#include "Foci/Components/Movement/EnemyMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

AEnemy::AEnemy(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	RootComponent = Capsule;

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
	SkeletalMesh->SetupAttachment(RootComponent);

	MovementComponent = CreateDefaultSubobject<UEnemyMovementComponent>(TEXT("Movement Component"));
	MovementComponent->SetUpdatedComponent(RootComponent);

	DropTable = CreateDefaultSubobject<UDropTable>(TEXT("Drop Table"));

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health Component"));
	HealthComponent->OnDeath.AddDynamic(this, &AEnemy::OnDeath);
}

UHealthComponent* AEnemy::GetHealthComponent() const
{
	return HealthComponent;
}

UCapsuleComponent* AEnemy::GetCapsule() const
{
	return Capsule;
}

void AEnemy::MoveForward()
{
	AddMovementInput(GetActorForwardVector());
}

void AEnemy::OnDeath()
{
	DropTable->SpawnDrops(GetActorLocation(), GetActorRotation(), 500.0f);
}

void AEnemy::OnHit(APawn* Attacker)
{
	const FVector DeltaLocation = Attacker->GetActorLocation() - GetActorLocation();
	MovementComponent->AddImpulse(DeltaLocation.GetSafeNormal2D() * -300.0f + FVector(0.0f, 0.0f, 300.0f));
}
