#include "FloorButton.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h" 

#include "Foci/Foci.h"
#include "Foci/FociCharacter.h"

AFloorButton::AFloorButton(const FObjectInitializer& ObjectInitializer)
{
	ButtonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Button Mesh"));
	RootComponent = ButtonMesh;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetupAttachment(ButtonMesh, TEXT("Button_Top"));
	Box->OnComponentHit.AddDynamic(this, &AFloorButton::OnHit);
}

void AFloorButton::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	AFociCharacter* FociCharacter = Cast<AFociCharacter>(OtherActor);
	if (!FociCharacter)
	{
		return;
	}
	// UE_LOG(LogButtonSystem, Display, TEXT("FloorButton hit by '%s', direction is '%s'"), *OtherActor->GetFName().ToString(), *Hit.ImpactNormal.ToString())
	if (Hit.ImpactNormal.Z <= -0.9f)
	{
		Trigger(OtherActor);
	}
}

UBoxComponent* AFloorButton::GetSurface() const
{
	return Box;
}
