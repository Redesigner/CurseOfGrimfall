// Copyright 2023 Stephen Melnick

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pot.generated.h"

UCLASS()
class FOCI_API APot : public AActor
{
	GENERATED_BODY()
	
public:	
	APot();

public:	
	void Break();

	UFUNCTION(BlueprintImplementableEvent)
	void OnBreak();


private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Drops, meta = (AllowPrivateAccess = "true"))
	class UDropTable* DropTable;
};
