// Copyright Epic Games, Inc. All Rights Reserved.

#include "FociCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"

#include "Foci/Components/HitboxController.h"
#include "MarleMovementComponent.h"
#include "Ladder.h"
#include "Foci/Actors/Interactable.h"

//////////////////////////////////////////////////////////////////////////
// AFociCharacter

AFociCharacter::AFociCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMarleMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->OnComponentBeginOverlap.AddUniqueDynamic(this, &AFociCharacter::OnBeginOverlap);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	MarleMovementComponent = Cast<UMarleMovementComponent>(GetCharacterMovement());

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	InteractTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("Interact Trigger"));
	InteractTrigger->SetupAttachment(RootComponent);

	HitboxController = CreateDefaultSubobject<UHitboxController>(TEXT("Hitbox Controller"));
	HitboxController->SetupAttachment(GetMesh());
	HitboxController->HitDetectedDelegate.BindUObject(this, &AFociCharacter::HitTarget);
}

void AFociCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bMovingToLocation)
	{
		const FVector ActorLocation = GetActorLocation();
		if (FMath::IsNearlyEqual(ActorLocation.X, Destination.X, 10) && FMath::IsNearlyEqual(ActorLocation.Y, Destination.Y, 10))
		{
			bMovingToLocation = false;
			return;
		}
		AddMovementInput(Destination - GetActorLocation());
	}
	if (HasTarget())
	{
		LookAtTarget();
	}
}

void AFociCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

bool AFociCharacter::CanJumpInternal_Implementation() const
{
	return true;
}

void AFociCharacter::OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal,
	const FVector& PreviousFloorContactNormal, const FVector& PreviousLocation, float TimeDelta)
{
	const FVector Velocity = GetVelocity();
	const float VelocityXYSquared = Velocity.X * Velocity.X + Velocity.Y * Velocity.Y;
	const float AutoJumpVelocitySquared = AutoJumpVelocity * AutoJumpVelocity;

	if (VelocityXYSquared > AutoJumpVelocitySquared && CanJump())
	{
		Jump();
	}
}

void AFociCharacter::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA<ALadder>())
	{
		GrabLadder(Cast<ALadder>(OtherActor));
	}
}

void AFociCharacter::MoveToLocation(FVector Location)
{
	bMovingToLocation = true;
	Destination = Location;
}

void AFociCharacter::SetInputEnabled(bool bEnabled)
{
	bInputEnabled = bEnabled;
}

bool AFociCharacter::HasTarget() const
{
	return FocusTarget.IsValid();
}

void AFociCharacter::LookAtTarget()
{
	const FVector Difference = FocusTarget->GetActorLocation() - GetActorLocation();
	FRotator ActorRotation = GetActorRotation();

	const double OldYaw = ActorRotation.Yaw;
	const double NewYaw = FMath::RadiansToDegrees(FMath::Atan2(Difference.Y, Difference.X));
	const double YawDifference = FMath::UnwindDegrees(NewYaw - OldYaw);
	ActorRotation.Yaw = NewYaw;

	SetActorRotation(ActorRotation);

	FRotator CameraRotation = Controller->GetControlRotation();
	CameraRotation.Add(0.0, YawDifference, 0.0);
	Controller->SetControlRotation(CameraRotation);
}

void AFociCharacter::GrabLadder(ALadder* Ladder)
{
	MarleMovementComponent->GrabLadder(Ladder);
}

void AFociCharacter::Interact()
{
	TSet<AActor*> OverlappingActors;
	InteractTrigger->GetOverlappingActors(OverlappingActors);
	bool bFoundTarget = false;
	for (AActor* Actor : OverlappingActors)
	{
		if (Actor->ActorHasTag(TEXT("Targetable")))
		{
			bFoundTarget = true;
			SetFocusTarget(Actor);
			continue;
		}
		AInteractable* Interactable = Cast<AInteractable>(Actor);
		if (!Interactable)
		{
			continue;
		}
		Interactable->Interact(this);
	}
	if (!bFoundTarget)
	{
		ClearFocusTarget();
	}
}

void AFociCharacter::SetFocusTarget(AActor* Target)
{
	FocusTarget = Target;
	MarleMovementComponent->bOrientRotationToMovement = false;

	FRotator ActorRotation = GetActorRotation();

	const FVector Difference = FocusTarget->GetActorLocation() - GetActorLocation();
	const double NewYaw = FMath::RadiansToDegrees(FMath::Atan2(Difference.Y, Difference.X));
	ActorRotation.Yaw = NewYaw;
	SetActorRotation(ActorRotation);
}

void AFociCharacter::ClearFocusTarget()
{
	FocusTarget = nullptr;
	MarleMovementComponent->bOrientRotationToMovement = true;
}



//////////////////////////////////////////////////////////////////////////
// Input
void AFociCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		UE_LOG(LogTemp, Display, TEXT("Attaching PlayerInputComponent to %s"), *GetFName().ToString())

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFociCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFociCharacter::Look);

		//A-button equivalent
		EnhancedInputComponent->BindAction(PrimaryAction, ETriggerEvent::Triggered, this, &AFociCharacter::Primary);

		//B-button equivalent
		EnhancedInputComponent->BindAction(SecondaryAction, ETriggerEvent::Triggered, this, &AFociCharacter::Secondary);

	}

}

void AFociCharacter::Move(const FInputActionValue& Value)
{
	if (!bInputEnabled)
	{
		return;
	}
	if (!Controller)
	{
		return;
	}
	if (bMovingToLocation)
	{
		return;
	}

	FVector2D MovementVector = Value.Get<FVector2D>();
	if (MarleMovementComponent && MarleMovementComponent->UseDirectInput())
	{
		AddMovementInput(FVector::ForwardVector, MovementVector.Y);
		AddMovementInput(FVector::RightVector, MovementVector.X);
		return;
	}
	if (FocusTarget.IsValid())
	{
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
		return;
	}
	// find out which way is forward
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void AFociCharacter::Look(const FInputActionValue& Value)
{
	if (!bInputEnabled)
	{
		return;
	}
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AFociCharacter::Primary(const FInputActionValue& Value)
{
	if (!bInputEnabled)
	{
		return;
	}
	if (MarleMovementComponent->MovementMode == EMovementMode::MOVE_Custom)
	{
		MarleMovementComponent->JumpAction();
		return;
	}
	Interact();
}

void AFociCharacter::Secondary(const FInputActionValue& Value)
{
	Attack();
}