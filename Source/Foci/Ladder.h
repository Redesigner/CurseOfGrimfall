// Copyright 2023 Stephen Melnick

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Ladder.generated.h"

UCLASS()
class FOCI_API ALadder : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class USplineComponent* SplineComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* TriggerVolume;

	
public:	
	// Sets default values for this actor's properties
	ALadder();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FVector GetGrabLocation(const float InputKey) const;

	float GetGrabDistance(const FVector StartLocation) const;

	bool IsEndOfSpline(const float InputKey) const;

	bool IsTop(const float InputKey) const;

	bool IsBottom(const float InputKey) const;

	FRotator GetGrabRotation(const FVector StartLocation) const;

};
