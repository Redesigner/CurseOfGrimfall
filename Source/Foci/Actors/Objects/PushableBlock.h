#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PushableBlock.generated.h"

UCLASS()
class FOCI_API APushableBlock : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* Box;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	FVector Velocity;
	
public:
	APushableBlock();

	virtual void Tick(float DeltaTime) override;

	void Push(FVector Delta, AActor* Source, FHitResult& HitResult);

	class UBoxComponent* GetBlockComponent() const;

private:
	bool SnapToFloor();

	bool SnapToButton(class AFloorButton* FloorButton, FVector Delta);

	TWeakObjectPtr<AActor> LastGrabber;
};
