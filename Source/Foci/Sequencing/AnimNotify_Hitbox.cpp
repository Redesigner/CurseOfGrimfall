// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_Hitbox.h"

#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"

#include "Foci/FociCharacter.h"
#include "Foci/Components/HitboxController.h"


void UAnimNotify_Hitbox::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	if (!MeshComp->GetOwner())
	{
		return;
	}
	UActorComponent* HitboxControllerComponent = MeshComp->GetOwner()->GetComponentByClass(UHitboxController::StaticClass());
	if (UHitboxController* HitboxController = Cast<UHitboxController>(HitboxControllerComponent))
	{
		// We need to somehow get the current ability from this context...
		HitboxController->SpawnHitbox(HitboxName, ArmorInstance.Position, ArmorInstance.Rotation, ArmorInstance.Normal, ArmorInstance.Dimensions, ArmorInstance.Shape);
	}
	if (UWorld* World = MeshComp->GetWorld())
	{
		if (World->WorldType == EWorldType::EditorPreview)
		{
			if (VisualizerShape)
			{
				VisualizerShape->DestroyComponent();
			}
			switch (ArmorInstance.Shape)
			{
				case Sphere:
				{
					USphereComponent* VisualizerSphere = NewObject<USphereComponent>(World, FName(TEXT("Sphere") + HitboxName.ToString()) );
					VisualizerSphere->SetSphereRadius(ArmorInstance.Dimensions.X);
					VisualizerShape = VisualizerSphere;
					break;
				}
				case Capsule:
				{
					UCapsuleComponent* Capsule = NewObject<UCapsuleComponent>(World, FName(TEXT("Capsule") + HitboxName.ToString()) );
					Capsule->SetCapsuleSize(ArmorInstance.Dimensions.X, ArmorInstance.Dimensions.Y);
					VisualizerShape = Capsule;
					break;
				}
				case Box:
				{
					UBoxComponent* Box = NewObject<UBoxComponent>(World, FName(TEXT("Box") + HitboxName.ToString()) );
					Box->SetBoxExtent(ArmorInstance.Dimensions);
					VisualizerShape = Box;
				}
			}
			VisualizerShape->SetWorldLocation(ArmorInstance.Position);
			VisualizerShape->SetWorldRotation(FRotator::MakeFromEuler(ArmorInstance.Rotation));
			VisualizerShape->SetVisibility(true);
			VisualizerShape->RegisterComponentWithWorld(World);
		}
	}
}


void UAnimNotify_Hitbox::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp->GetOwner())
	{
		return;
	}
	UActorComponent* HitboxControllerComponent = MeshComp->GetOwner()->GetComponentByClass(UHitboxController::StaticClass());
	if (UHitboxController* HitboxController = Cast<UHitboxController>(HitboxControllerComponent))
	{
		HitboxController->RemoveHitboxByName(HitboxName);
	}
	if (VisualizerShape)
	{
		VisualizerShape->DestroyComponent();
	}
}

#if WITH_EDITOR
void UAnimNotify_Hitbox::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	/*
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UAnimNotify_Hitbox, HitboxDirection))
	{
		HitboxDirection = HitboxDirection.GetSafeNormal();
	}
	*/
}
#endif
