// Fill out your copyright notice in the Description page of Project Settings.


#include "ArmorComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"


UArmorComponent::UArmorComponent(const FObjectInitializer& ObjectInitializer)
    :Super(ObjectInitializer)
{
}

void UArmorComponent::DestroyComponent(bool bPromoteChildren)
{
    if (DirectionalArrow)
    {
        DirectionalArrow->DestroyComponent();
    }
    if (Shape)
    {
        Shape->DestroyComponent();
    }
    Super::DestroyComponent(bPromoteChildren);
}

void UArmorComponent::SpawnShape(EArmorShape ShapeType, FVector Dimensions)
{
    // The shape components are hardcoded **only** to be easier to manipulate in the editor
    const FName ShapeName = TEXT("Shape");
    switch (ShapeType)
    {
        case Sphere:
        {
            USphereComponent* Sphere = NewObject<USphereComponent>(this, ShapeName);
            Sphere->SetSphereRadius(Dimensions.X);
            Shape = Sphere;
            break;
        }
        case Capsule:
        {
            UCapsuleComponent* Capsule = NewObject<UCapsuleComponent>(this, ShapeName);
            Capsule->SetCapsuleSize(Dimensions.X, Dimensions.Y);
            Shape = Capsule;
            break;
        }
        case Box:
        {
            UBoxComponent* Box = NewObject<UBoxComponent>(this, ShapeName);
            Box->SetBoxExtent(Dimensions);
            Shape = Box;
            break;
        }
    }
    Shape->ShapeColor = Color;
    Shape->SetVisibility(IsVisible());
    Shape->SetHiddenInGame(false);
    Shape->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
    Shape->RegisterComponent();
}

bool UArmorComponent::CanBlockHit(const FGameplayAbilitySpec* HitSpec) const
{
    // @TODO: Compare levels of armor and attack correctly... for now all attacks will be blocked
    return true;
}

void UArmorComponent::SetNormal(const FVector& InNormal)
{
    Normal = InNormal;
#if WITH_EDITOR
    DirectionalArrow = NewObject<UArrowComponent>(this, TEXT("Directional Arrow"));
    DirectionalArrow->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
    DirectionalArrow->SetVisibility(IsVisible());
    DirectionalArrow->SetHiddenInGame(false);
    DirectionalArrow->RegisterComponent();
    DirectionalArrow->SetRelativeRotation(Normal.ToOrientationRotator());
    DirectionalArrow->SetArrowColor(Color);
    DirectionalArrow->ArrowLength = 25.0f;
#endif

}

FVector UArmorComponent::GetNormal() const
{
    return Normal;
}

void UArmorComponent::SetCollisionProfileName(FName CollisionProfileName)
{
    Shape->SetCollisionProfileName(CollisionProfileName, false);
}

void UArmorComponent::SetDimensions(FVector Dimensions)
{
    switch (ArmorShape)
    {
        case Sphere:
        {
            Cast<USphereComponent>(Shape)->SetSphereRadius(Dimensions.X);
            break;
        }
        case Capsule:
        {
            Cast<UCapsuleComponent>(Shape)->SetCapsuleSize(Dimensions.X, Dimensions.Y);
            break;
        }
        case Box:
        {
            Cast<UBoxComponent>(Shape)->SetBoxExtent(Dimensions);
            break;
        }
    }
}

UShapeComponent* UArmorComponent::GetShapeComponent()
{
    return Shape;
}
