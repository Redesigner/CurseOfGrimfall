#include "Enemy.h"

#include "Foci/Actors/DropTable.h"
#include "Foci/Actors/EnemyAIController.h"
#include "Foci/Components/HealthComponent.h"
#include "Foci/Components/Movement/EnemyMovementComponent.h"
#include "Foci/Components/HitboxController.h"

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

	HitboxController = CreateDefaultSubobject<UHitboxController>(TEXT("Hitbox Controller"));
	HitboxController->SetupAttachment(SkeletalMesh);
	HitboxController->HitDetectedDelegate.BindUObject(this, &AEnemy::HitTarget_Internal);

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

UEnemyMovementComponent* AEnemy::GetEnemyMovementComponent() const
{
	return MovementComponent;
}

const USkeletalMeshComponent* AEnemy::GetSkeletalMeshComponent() const
{
	return SkeletalMesh;
}

void AEnemy::MoveForward()
{
	AddMovementInput(GetActorForwardVector());
}

void AEnemy::OnDeath()
{
	DropTable->SpawnDrops(GetActorLocation(), GetActorRotation(), 300.0f);
	MovementComponent->SetMovementMode(EEnemyMovementMode::MOVE_None);
	SetLifeSpan(3.0f);
	Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Death();
}

void AEnemy::Attack()
{
	if (!HealthComponent->IsAlive() || MovementComponent->IsFalling())
	{
		return;
	}
	SkeletalMesh->GetAnimInstance()->Montage_Play(AttackMontage);
}

void AEnemy::HitTarget_Internal(const FHitResult& HitResult)
{
	UActorComponent* Component = HitResult.GetActor()->GetComponentByClass(UHealthComponent::StaticClass());
	if (!Component)
	{
		return;
	}
	UHealthComponent* TargetHealthComponent = Cast<UHealthComponent>(Component);
	TargetHealthComponent->AddHealth(-AttackDamage);
}

void AEnemy::OnHit(APawn* Attacker)
{
	const FVector DeltaLocation = Attacker->GetActorLocation() - GetActorLocation();
	MovementComponent->AddImpulse(DeltaLocation.GetSafeNormal2D() * -300.0f + FVector(0.0f, 0.0f, 300.0f));
	
	if (AEnemyAIController* EnemyAI = Cast<AEnemyAIController>(GetController()))
	{
		// Dont let them get away with it!!!
		EnemyAI->TrySetTarget(Attacker);
	}
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	HealthComponent->OnDeath.AddDynamic(this, &AEnemy::OnDeath);
}

void AEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetActorLocation().Z <= -500.0f)
	{
		HealthComponent->AddHealth(-3.0f);
	}
}
