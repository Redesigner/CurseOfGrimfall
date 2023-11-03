#include "Door.h"

#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "ActorSequenceComponent.h" 
#include "ActorSequencePlayer.h"
#include "Foci/FociCharacter.h"
#include "Foci/FociGameMode.h"

ADoor::ADoor()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door Mesh"));
	DoorMesh->SetupAttachment(RootComponent);

	EnterLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Enter Location"));
	EnterLocation->SetupAttachment(RootComponent);

	ExitLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Exit Location"));
	ExitLocation->SetupAttachment(RootComponent);

	GameMode = nullptr;
}

void ADoor::BeginPlay()
{
	Super::BeginPlay();
	DoorInitialOffset = DoorMesh->GetRelativeLocation();
	GameMode = Cast<AFociGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
}

void ADoor::Tick(float DeltaTime)
{
	FVector DoorDesiredLocation = bDoorOpen ? DoorInitialOffset + DoorOpenOffset : DoorInitialOffset;
	FVector DoorOffset = FMath::VInterpConstantTo(DoorMesh->GetRelativeLocation(), DoorDesiredLocation, DeltaTime, DoorMovementSpeed);
	DoorMesh->SetRelativeLocation(DoorOffset);
}

void ADoor::Interact(AFociCharacter* Source)
{
	FlipDoor(Source->GetActorLocation());

	if (GameMode)
	{
		GameMode->bWorldActive = false;
		GameMode->LastSpawnLocation = ExitLocation->GetComponentLocation();
		GameMode->LastSpawnRotation = FRotator(0.0f, ExitLocation->GetComponentRotation().Yaw, 0.0f);
	}

	bDoorOpen = true;
	if (MovingSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), MovingSound);
	}
	APlayerController* PlayerController = Cast<APlayerController>(Source->GetController());

	FViewTargetTransitionParams TransitionParams;
	TransitionParams.BlendTime = 0.5f;

	PlayerController->SetViewTarget(this, TransitionParams);
	Source->MoveToLocation(EnterLocation->GetComponentLocation());
	Source->SetInputEnabled(false);
	
	FTimerDelegate SequenceEndDelegate;
	SequenceEndDelegate.BindLambda([this, Source, PlayerController, TransitionParams]() {
		Source->ResetCameraRotation();
		Source->SetInputEnabled(true);
		PlayerController->SetViewTarget(Source, TransitionParams);
		GameMode->bWorldActive = true;
		});

	FTimerHandle DoorCloseTimer;
	FTimerDelegate DoorCloseDelegate;
	DoorCloseDelegate.BindLambda([this]() {
		if (MovingSound)
		{
			UGameplayStatics::PlaySound2D(GetWorld(), MovingSound);
		}
		bDoorOpen = false;
		});

	FTimerHandle PlayerMoveTimer;
	FTimerDelegate PlayerMoveDelegate;
	PlayerMoveDelegate.BindLambda([Source, this]() {
		Source->MoveToLocation(ExitLocation->GetComponentLocation());
		});

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.SetTimer(DoorSequenceTimer, SequenceEndDelegate, 4.0f, false);
	TimerManager.SetTimer(DoorCloseTimer, DoorCloseDelegate, 3.0f, false);
	TimerManager.SetTimer(PlayerMoveTimer, PlayerMoveDelegate, 2.0f, false);
}

void ADoor::FlipDoor(FVector PlayerLocation)
{
	FVector DeltaLocation = PlayerLocation - GetActorLocation();
	if (DeltaLocation.Dot(GetActorForwardVector()) > 0.0f)
	{
		// FTransform DoorMeshTransform = DoorMesh->GetComponentTransform();
		AddActorWorldRotation(FRotator(0.0f, 180.0f, 0.0f));
		// DoorMesh->SetWorldTransform(DoorMeshTransform);
	}
}
