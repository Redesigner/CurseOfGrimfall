// Fill out your copyright notice in the Description page of Project Settings.


#include "HitboxController.h"

#include "Foci\Components\ArmorComponent.h"

void UHitboxController::BeginPlay()
{
	Super::BeginPlay();
}

TWeakObjectPtr<UArmorComponent> UHitboxController::GetHitboxByName(FName Name)
{
	for (USceneComponent* SceneComponent : GetAttachChildren())
	{
		if (UArmorComponent* SphereComponent = Cast<UArmorComponent>(SceneComponent))
		{
			if (SphereComponent->GetFName() == Name)
			{
				return SphereComponent;
			}
		}
	}
	return nullptr;
}

TWeakObjectPtr<UArmorComponent> UHitboxController::SpawnHitbox(FName Name, FVector HitboxRelativeLocation, FVector Rotation, FVector Direction, FVector Dimensions, EArmorShape Shape)
{
	UArmorComponent* Hitbox = NewObject<UArmorComponent>(this, Name);
	Hitbox->Color = FColor::Red;

	Hitbox->SpawnShape(Shape, Dimensions);
	Hitbox->GetShapeComponent()->OnComponentBeginOverlap.AddDynamic(this, &UHitboxController::AccumulateOverlaps);

	Hitbox->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	Hitbox->SetRelativeLocation(HitboxRelativeLocation, false, nullptr, ETeleportType::ResetPhysics);
	Hitbox->SetNormal(Direction);
	Hitbox->ArmorShape = Shape;
	Hitbox->OwningAbility = CurrentAbilitySpec;
	Hitbox->SetRelativeRotation(FRotator::MakeFromEuler(Rotation));

	Hitbox->SetVisibility(IsVisible(), true);
	Hitbox->SetHiddenInGame(false);

	Hitbox->SetCollisionProfileName(HitboxCollisionProfile.Name);
	Hitbox->GetShapeComponent()->SetGenerateOverlapEvents(true);
	Hitbox->RegisterComponent();

	// Trigger any overlap events that happened when we spawned the hitbox
	Hitbox->GetShapeComponent()->UpdateOverlaps();
	return Hitbox;
}

void UHitboxController::RemoveHitboxByName(FName Name)
{
	TWeakObjectPtr<UArmorComponent> Hitbox = GetHitboxByName(Name);
	if (Hitbox.IsValid())
	{
		Hitbox->DestroyComponent();
	}
}

TWeakObjectPtr<UArmorComponent> UHitboxController::GetArmorByName(FName Name)
{
	for (USceneComponent* SceneComponent : GetAttachChildren())
	{
		if (UArmorComponent* SphereComponent = Cast<UArmorComponent>(SceneComponent))
		{
			if (SphereComponent->GetFName() == Name)
			{
				return SphereComponent;
			}
		}
	}
	return nullptr;
}

TWeakObjectPtr<UArmorComponent> UHitboxController::SpawnArmor(FName Name, FVector ArmorRelativeLocation, FVector Rotation, FVector Normal, FVector Dimensions, EArmorShape Shape)
{
	UArmorComponent* Armor = NewObject<UArmorComponent>(this, Name);
	Armor->Color = FColor::Blue;
	Armor->SpawnShape(Shape, Dimensions);
	Armor->OwningHitboxController = this;
	Armor->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	Armor->SetNormal(Normal);
	Armor->SetRelativeLocation(ArmorRelativeLocation, false, nullptr, ETeleportType::ResetPhysics);
	Armor->SetRelativeRotation(FRotator::MakeFromEuler(Rotation));

	Armor->SetVisibility(IsVisible(), true);
	Armor->SetHiddenInGame(false);

	Armor->SetCollisionProfileName(ArmorCollisionProfile.Name);

	Armor->RegisterComponent();
	/*
	* UE_LOG(LogTemp, Display, TEXT("Spawned armor hitbox for '%s' with collision profile '%s'. Responding with '%s' to objects on the armor collision channel '%s'"),
	*	GetOwner()->GetFName().ToString(), *ArmorCollisionProfile.Name.ToString(),
	*	UEnum::GetDisplayValueAsText<ECollisionResponse>(Armor->GetCollisionResponseToChannel(BlockCollisionChannel)).ToString(),
	*	UEnum::GetDisplayValueAsText<ECollisionChannel>(BlockCollisionChannel).ToString() )
	*/
	return Armor;
}

void UHitboxController::RemoveArmorByName(FName Name)
{
	TWeakObjectPtr<UArmorComponent> Armor = GetArmorByName(Name);
	if (Armor.IsValid())
	{
		Armor->DestroyComponent();
	}
}

void UHitboxController::AccumulateOverlaps(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (OtherActor == GetOwner())
	{
		return;
	}
	for (const FHitResult& HitResult : QueuedHits)
	{
		// Only accumulate the hit if the target actor has not already been hit in this ability.
		// Disable this check for multi-hit attacks
		if (OtherActor == HitResult.GetActor())
		{
			return;
		}
	}
	FHitResult ModifiedHit = FHitResult(SweepResult);
	// ModifiedHit.Location = OverlappedComponent->GetComponentLocation();
	ModifiedHit.Location = OtherComp->GetComponentLocation();
	
	if (UArmorComponent* HitboxComponent = Cast<UArmorComponent>(OverlappedComponent->GetAttachParent() ))
	{
		ModifiedHit.Normal =  HitboxComponent->GetComponentRotation().RotateVector(HitboxComponent->GetNormal() );

		if (GetIsHitBlocked(HitboxComponent, OtherActor->GetActorLocation() ))
		{
			HitInterruptedDelegate.ExecuteIfBound(ModifiedHit);
			return;
		}
	}
	HitDetectedDelegate.ExecuteIfBound(ModifiedHit);
	QueuedHits.Emplace(ModifiedHit);
}


void UHitboxController::ClearOverlaps()
{
	QueuedHits.Empty();
}

void UHitboxController::SetCurrentGameplayAbilitySpec(const FGameplayAbilitySpec* AbilitySpec)
{
	CurrentAbilitySpec = AbilitySpec;
}


bool UHitboxController::GetIsHitBlocked(const UArmorComponent* Hitbox, const FVector& HitLocation) const
{
	FHitResult ArmorTestResult;
	FCollisionObjectQueryParams QueryParams;
	QueryParams.AddObjectTypesToQuery(ECC_GameTraceChannel1);
	FCollisionQueryParams CollisionQueryParams;
	// const FVector StartLocation = Hitbox->GetComponentLocation();
	const FVector StartLocation = GetOwner()->GetActorLocation();
	const FVector EndLocation = HitLocation;
	GetWorld()->LineTraceSingleByChannel(ArmorTestResult, StartLocation, EndLocation, BlockCollisionChannel);
	// GetWorld()->LineTraceSingleByObjectType(ArmorTestResult, StartLocation, EndLocation, QueryParams);
	// We hit some armor!
	// UE_LOG(LogTemp, Display, TEXT("Checking if hit blocked. Sweeping along trace channel '%s"), *UEnum::GetDisplayValueAsText<ECollisionChannel>(BlockCollisionChannel).ToString())
	// DrawDebugDirectionalArrow(GetWorld(), StartLocation, EndLocation, 3.0f, FColor::Red, false, 2.0f);
	if (ArmorTestResult.bBlockingHit)
	{
		USceneComponent* ParentComponent = ArmorTestResult.GetComponent()->GetAttachParent();
		if (UArmorComponent* ArmorComponent = Cast<UArmorComponent>(ParentComponent))
		{
			// Convert our armor's built-in impact normal to world space
			ArmorTestResult.Normal = ArmorComponent->GetComponentRotation().RotateVector(ArmorComponent->GetNormal());
			ArmorComponent->OwningHitboxController->HitBlockedDelegate.ExecuteIfBound(ArmorTestResult);
			// Hand this check off to the component itself
			return ArmorComponent->CanBlockHit(Hitbox->OwningAbility);
		}
	}
	return false;
}
