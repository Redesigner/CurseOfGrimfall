// Copyright Epic Games, Inc. All Rights Reserved.

#include "FociCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "Kismet/GameplayStatics.h" 

#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"

#include "Foci.h"
#include "Dialog/DialogViewModel.h"
#include "Foci/Components/HitboxController.h"
#include "MarleMovementComponent.h"
#include "Ladder.h"
#include "Foci/Actors/Interactable.h"
#include "Foci/Actors/Enemy.h"
#include "Foci/Actors/Objects/PushableBlock.h"
#include "Foci/Components/WeaponTool.h"
#include "InteractableInterface.h"
#include "Foci/DialogComponent.h"
#include "Foci/Components/HealthComponent.h"
#include "Foci/Inventory/InventoryTable.h"
#include "Foci/Inventory/Pickup.h"

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
	MarleMovementComponent->OnMovementModeUpdated.AddDynamic(this, &AFociCharacter::OnMovementModeUpdated);

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(RootComponent);
	FirstPersonCamera->bUsePawnControlRotation = true;

	InteractTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("Interact Trigger"));
	InteractTrigger->SetupAttachment(RootComponent);

	HitboxController = CreateDefaultSubobject<UHitboxController>(TEXT("Hitbox Controller"));
	HitboxController->SetupAttachment(GetMesh());
	HitboxController->HitDetectedDelegate.BindUObject(this, &AFociCharacter::HitTarget_Internal);

	ViewMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ViewMesh"));
	ViewMesh->SetupAttachment(FirstPersonCamera);

	ShieldMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Shield Mesh"));
	ShieldMesh->SetupAttachment(GetMesh(), TEXT("Handle_R"));
	ShieldMesh->SetUsingAbsoluteScale(true);

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->OnDeath.AddDynamic(this, &AFociCharacter::OnDeath_Internal);

	DialogViewModel = CreateDefaultSubobject<UDialogViewModel>(TEXT("Dialog Viewmodel"));
	DialogViewModel->SetModel(this);
	DialogViewModel->HealthChanged(HealthComponent->GetCurrentHealth(), 1.0f);
	DialogViewModel->MaxHealthChanged(HealthComponent->GetMaxHealth());

	Inventory = CreateDefaultSubobject<UInventoryTable>(TEXT("Inventory"));
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
		if (!FocusTarget.IsValid() || !FocusTarget->GetHealthComponent()->IsAlive())
		{
			ClearFocusTarget();
			return;
		}
		LookAtTarget();
	}
}

void AFociCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	PlayerController = Cast<APlayerController>(NewController);
}

void AFociCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (PlayerController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	// Bind the viewmodel here, just to be safe, and refresh the health state
	HealthComponent->OnHealthChanged.AddDynamic(DialogViewModel, &UDialogViewModel::HealthChanged);
	HealthComponent->OnMaxHealthChanged.AddDynamic(DialogViewModel, &UDialogViewModel::MaxHealthChanged);

	Inventory->OnInventoryItemCountChanged.AddDynamic(DialogViewModel, &UDialogViewModel::InventoryItemCountChanged);
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
	if (!OtherActor)
	{
		return;
	}

	if (ALadder* Ladder = Cast<ALadder>(OtherActor))
	{
		GrabLadder(Ladder);
	}

	if (APickup* Pickup = Cast<APickup>(OtherActor))
	{
		if (Pickup->Pickup(this))
		{
			Inventory->GiveItem(Pickup->GetItemName(), Pickup->GetItemCount());
		}
	}
}

void AFociCharacter::OnMovementModeUpdated(EMovementMode NewMovementMode, uint8 NewCustomMode, EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	if (PreviousMovementMode == EMovementMode::MOVE_Walking)
	{
		DisableFirstPerson();
		ReleaseWeapon();
	}
}

void AFociCharacter::MoveToLocation(FVector Location)
{
	bMovingToLocation = true;
	Destination = Location;
}

void AFociCharacter::TryDisableFirstPerson()
{
	DisableFirstPerson();
}

void AFociCharacter::SetInputEnabled(bool bEnabled)
{
	bInputEnabled = bEnabled;
}

void AFociCharacter::ResetCameraRotation()
{
	Controller->SetControlRotation(GetActorRotation());
}

bool AFociCharacter::HasTarget() const
{
	return bHasFocusTarget;
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


///////////////////////////////////////////////////////////////////////////
// Interaction
void AFociCharacter::Interact()
{
	if (!bInputEnabled)
	{
		return;
	}
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	if (TimerManager.IsTimerActive(InteractDebounceTimer))
	{
		return;
	}
	TimerManager.SetTimer(InteractDebounceTimer, InteractDebounceTime, false);

	TSet<AActor*> OverlappingActors;
	InteractTrigger->GetOverlappingActors(OverlappingActors);
	bool bFoundTarget = false;
	for (AActor* Actor : OverlappingActors)
	{
		if (AEnemy* Enemy = Cast<AEnemy>(Actor))
		{
			bFoundTarget = true;
			SetFocusTarget(Enemy);
			continue;
		}
		if (UActorComponent* DialogActorComponent = Actor->GetComponentByClass(UDialogComponent::StaticClass()) )
		{
			LastInteractedNPC = Cast<UDialogComponent>(DialogActorComponent);
			LastInteractedNPC->RequestDialog(this, FDialogRequest());
			continue;
		}
		if (IInteractableInterface* Interactable = Cast<IInteractableInterface>(Actor))
		{
			Interactable->Interact(this);
			continue;
		}
		if (APushableBlock* Block = Cast<APushableBlock>(Actor))
		{
			MarleMovementComponent->GrabBlock(Block);
			continue;
		}
	}
	if (!bFoundTarget && bHasFocusTarget)
	{
		ClearFocusTarget();
	}
}
///////////////////////////////////////////////////////////////////////////



void AFociCharacter::SetFocusTarget(AEnemy* Target)
{
	bHasFocusTarget = true;
	FocusTarget = Target;
	MarleMovementComponent->bOrientRotationToMovement = false;
	FRotator ActorRotation = GetActorRotation();

	const FVector Difference = FocusTarget->GetActorLocation() - GetActorLocation();
	const double NewYaw = FMath::RadiansToDegrees(FMath::Atan2(Difference.Y, Difference.X));
	ActorRotation.Yaw = NewYaw;
	SetActorRotation(ActorRotation);

	DisableFirstPerson(); 
	// ReleaseWeapon();
}

void AFociCharacter::ClearFocusTarget()
{
	bHasFocusTarget = false;
	FocusTarget = nullptr;
	MarleMovementComponent->bOrientRotationToMovement = true;
	if (bWeaponReady)
	{
		ReleaseWeapon();
	}
}

bool AFociCharacter::GetFirstPerson() const
{
	return bFirstPersonMode;
}

bool AFociCharacter::IsWeaponDrawn() const
{
	return bWeaponDrawn;
}

bool AFociCharacter::IsWeaponReady() const
{
	return bWeaponReady;
}


UInventoryTable* AFociCharacter::GetInventory()
{
	return Inventory;
}

void AFociCharacter::OnDeath_Internal()
{
	if (bImmortal) { return; }
	// UGameplayStatics::SetGamePaused(GetWorld(), true);
	MarleMovementComponent->SetMovementMode(MOVE_None);
	SetInputEnabled(false);
	OnDeath();
}

UHealthComponent* AFociCharacter::GetHealthComponent()
{
	return HealthComponent;
}


FRotator AFociCharacter::GetHandDirection() const
{
	return HandDirection;
}

void AFociCharacter::EnableFirstPerson()
{
	FirstPersonCamera->Activate();
	FollowCamera->Deactivate();
	bFirstPersonMode = true;
	MarleMovementComponent->bUseControllerDesiredRotation = true;
	MarleMovementComponent->bOrientRotationToMovement = false;
	GetMesh()->SetVisibility(false);
	ViewMesh->SetVisibility(true);

	if (CurrentWeapon)
	{
		CurrentWeapon->SetFirstPerson();
	}
}


void AFociCharacter::DisableFirstPerson()
{
	FollowCamera->Activate();
	FirstPersonCamera->Deactivate();
	bFirstPersonMode = false;
	MarleMovementComponent->bUseControllerDesiredRotation = false;
	if (!HasTarget())
	{
		MarleMovementComponent->bOrientRotationToMovement = true;
	}
	GetMesh()->SetVisibility(true);
	ViewMesh->SetVisibility(false);

	if (CurrentWeapon)
	{
		CurrentWeapon->SetThirdPerson();
	}
}

void AFociCharacter::RaiseShield()
{
	bBlocking = true;
	const FVector ShieldOffset = FVector(0.0f, 70.0f, 100.0f);
	const FVector ShieldRotator;
	const FVector Normal = FVector(0.0f, 1.0f, 0.0f);
	const FVector ShapeDimensions = FVector(50.0f, 70.0f, 0.0f);
	HitboxController->SpawnArmor(TEXT("Shield"), ShieldOffset, ShieldRotator, Normal, ShapeDimensions, EArmorShape::Capsule);
	// ShieldMesh->SetVisibility(true);
}

void AFociCharacter::LowerShield()
{
	// ShieldMesh->SetVisibility(false);
	HitboxController->RemoveHitboxByName(TEXT("Shield"));
	bBlocking = false;
}

bool AFociCharacter::CanAttack() const
{
	return !bAttacking && MarleMovementComponent->MovementMode == EMovementMode::MOVE_Walking;
}


void AFociCharacter::Attack()
{
	if (!CanAttack())
	{
		return;
	}
	// Force the shield to lower, even if the button is held
	if (bBlocking)
	{
		LowerShield();
	}

	bAttacking = true;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->Montage_Play(AttackMontage);
	AnimInstance->OnMontageEnded.AddDynamic(this, &AFociCharacter::OnAttackMontageEnded);
}

void AFociCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bAttacking = false;
	// If the shield is lowered, but we're holding the button, bring it back up.
	if (bShieldHeld)
	{
		RaiseShield();
	}
}


void AFociCharacter::HitTarget_Internal(const FHitResult& HitResult)
{
	if (AEnemy* Enemy = Cast<AEnemy>(HitResult.GetActor()))
	{
		Enemy->GetHealthComponent()->AddHealth(-1.0f);
		Enemy->OnHit(this);
	}
	HitTarget(HitResult);
}


void AFociCharacter::SetFirstPerson(bool bFirstPerson)
{
	if (HasTarget())
	{
		return;
	}
	if (bFirstPerson && !bFirstPersonMode)
	{
		EnableFirstPerson();
		return;
	}
	if (!bFirstPerson && bFirstPersonMode)
	{
		DisableFirstPerson();
		return;
	}
}


//////////////////////////////////////////////////////////////////////////
// Input
void AFociCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		// UE_LOG(LogTemp, Display, TEXT("Attaching PlayerInputComponent to %s"), *GetFName().ToString())

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFociCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFociCharacter::Look);

		//A-button equivalent
		EnhancedInputComponent->BindAction(PrimaryAction, ETriggerEvent::Started, this, &AFociCharacter::PrimaryPressed);
		EnhancedInputComponent->BindAction(PrimaryAction, ETriggerEvent::Completed, this, &AFociCharacter::PrimaryReleased);

		//B-button equivalent
		EnhancedInputComponent->BindAction(SecondaryAction, ETriggerEvent::Started, this, &AFociCharacter::Secondary);

		EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Started, this, &AFociCharacter::BlockPressed);
		EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Completed, this, &AFociCharacter::BlockReleased);


		//Slot1
		EnhancedInputComponent->BindAction(Slot1Action, ETriggerEvent::Started, this, &AFociCharacter::Slot1Pressed);
		EnhancedInputComponent->BindAction(Slot1Action, ETriggerEvent::Completed, this, &AFociCharacter::Slot1Released);

		//Slot2
		EnhancedInputComponent->BindAction(Slot2Action, ETriggerEvent::Started, this, &AFociCharacter::Slot2Pressed);
		EnhancedInputComponent->BindAction(Slot2Action, ETriggerEvent::Completed, this, &AFociCharacter::Slot2Released);
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
	if (MarleMovementComponent && MarleMovementComponent->UsingDirectInput())
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

void AFociCharacter::PrimaryPressed(const FInputActionValue& Value)
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

void AFociCharacter::PrimaryReleased(const FInputActionValue& Value)
{
	if (MarleMovementComponent->MovementMode == EMovementMode::MOVE_Custom && MarleMovementComponent->CustomMovementMode == 2)
	{
		MarleMovementComponent->ReleaseBlock();
	}
}


void AFociCharacter::Secondary(const FInputActionValue& Value)
{
	if (!bInputEnabled)
	{
		return;
	}
	if (bFirstPersonMode)
	{
		DisableFirstPerson();
		if (bWeaponReady)
		{
			ReleaseWeapon();
		}
		return;
	}
	if (bWeaponReady)
	{
		ReleaseWeapon();
		return;
	}
	Attack();
}

void AFociCharacter::BlockPressed(const FInputActionValue& Value)
{
	bShieldHeld = true;
	RaiseShield();
}

void AFociCharacter::BlockReleased(const FInputActionValue& Value)
{
	bShieldHeld = false;
	LowerShield();
}

void AFociCharacter::Slot1Pressed(const FInputActionValue& Value)
{
	SlotPressed(0);
}

void AFociCharacter::Slot1Released(const FInputActionValue& Value)
{
	SlotReleased(0);
}

void AFociCharacter::Slot2Pressed(const FInputActionValue& Value)
{
	SlotPressed(1);
}

void AFociCharacter::Slot2Released(const FInputActionValue& Value)
{
	SlotReleased(1);
}



void AFociCharacter::SlotPressed(uint8 SlotIndex)
{
	if (!Weapons[SlotIndex])
	{
		UE_LOG(LogWeaponSystem, Warning, TEXT("Weapon slot '%i' is null. Check that the weapon has been assigned properly."), SlotIndex)
		return;
	}
	if (!CanAttack())
	{
		return;
	}

	// If we aren't targeting something, we want to enter first-person mode, first
	// third-person mode and using a slotted weapon/tool aren't compatible right now
	if (!HasTarget() && !bFirstPersonMode)
	{
		EnableFirstPerson();
		ReadyWeapon(Weapons[SlotIndex]);
		return;
	}
	// we have a target, but our weapon isn't ready, so ready it
	if (!bWeaponReady)
	{
		ReadyWeapon(Weapons[SlotIndex]);
	}
	if (!bWeaponDrawn)
	{
		if (CurrentWeapon)
		{
			CurrentWeapon->DrawWeapon(this);
		}
		bWeaponDrawn = true;
	}
}

void AFociCharacter::SlotReleased(uint8 SlotIndex)
{
	// UE_LOG(LogWeaponSystem, Display, TEXT("Weapon at slot '%i' fired!"), SlotIndex)
	if (!bWeaponDrawn || !bWeaponReady)
	{
		return;
	}
	if (!CurrentWeapon->IsA(Weapons[SlotIndex]))
	{
		ReleaseWeapon();
		ReadyWeapon(Weapons[SlotIndex]);
		return;
	}
	bWeaponDrawn = false;
	FireWeapon();
}

void AFociCharacter::ReadyWeapon(TSubclassOf<class AWeaponTool> Weapon)
{
	if (CurrentWeapon)
	{
		UE_LOG(LogWeaponSystem, Warning, TEXT("Attempted to ready a weapon before releasing the curent weapon."))
		return;
	}
	CurrentWeapon = Cast<AWeaponTool>(GetWorld()->SpawnActor(Weapon));
	CurrentWeapon->AttachToActor(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	CurrentWeapon->AttachComponentsToSockets(GetMesh(), ViewMesh, bFirstPersonMode, TEXT("Handle_R"));
	bWeaponReady = true;
}

void AFociCharacter::ReleaseWeapon()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->Destroy(true);
	}
	CurrentWeapon = nullptr;
	bWeaponReady = false;
}


void AFociCharacter::FireWeapon()
{
	if (!CurrentWeapon)
	{
		return;
	}
	UE_LOG(LogWeaponSystem, Display, TEXT("Firing weapon '%s'"), *CurrentWeapon->GetFName().ToString())
	CurrentWeapon->Fire(this, GetActorLocation() + FVector::UpVector * 35.0f,
		HasTarget() ? (FocusTarget->GetActorLocation() - GetActorLocation()).ToOrientationRotator() : GetControlRotation());
}


float AFociCharacter::GetTetherLength() const
{
	return TetherLength;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dialog
const FDialogResponse& AFociCharacter::GetDialog() const
{
	return CurrentDialog;
}

void AFociCharacter::SetDialog(FDialogResponse Dialog)
{
	CurrentDialog = Dialog;
	DialogViewModel->SetDialog(CurrentDialog);
	UGameplayStatics::SetGamePaused(GetWorld(), !Dialog.IsEmpty());
	
	if (Dialog.IsEmpty())
	{
		LastInteractedNPC = nullptr;
	}
}

void AFociCharacter::RequestDialogFromLastNpc(FDialogRequest DialogRequest)
{
	if (!LastInteractedNPC.IsValid())
	{
		return;
	}
	LastInteractedNPC->RequestDialog(this, DialogRequest);
}


/* This function is intended for the MVVM plugin
void AFociCharacter::BindViewModel()
{
	
	UMVVMSubsystem* MVVMSubsystem = GEngine->GetEngineSubsystem<UMVVMSubsystem>();
	if (!MVVMSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("Unable to locate the MVVM Subsystem"))
		return;
	}
	UMVVMViewModelCollectionObject* ViewModelCollection = MVVMSubsystem->GetGlobalViewModelCollection();
	UMVVMViewModelBase* ViewModelBase = ViewModelCollection->FindViewModelInstance(DialogViewModelContext);
	if (ViewModelBase)
	{
		DialogViewModel = Cast<UDialogViewModel>(ViewModelBase);
		return;
	}
	if (ViewModelCollection->AddViewModelInstance(DialogViewModelContext, NewObject<UDialogViewModel>()))
	{
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("Failed to locate viewmodel"))
	
} */