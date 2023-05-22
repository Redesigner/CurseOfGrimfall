#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectSpawner.generated.h"

UCLASS()
class FOCI_API AObjectSpawner : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category = Spawner, meta = (AllowPrivateAccess = true))
		TSubclassOf<AActor> ObjectToSpawn;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category = Spawner, meta = (AllowPrivateAccess = true))
		float RespawnTime = 5.0f;

	FTimerHandle RespawnTimer;

	TWeakObjectPtr<AActor> SpawnedObject;


protected:
	virtual void BeginPlay() override;


public:
	UFUNCTION(BlueprintCallable)
	void Spawn();

	UFUNCTION()
	void OnObjectDestroyed(AActor* DestroyedActor);
};