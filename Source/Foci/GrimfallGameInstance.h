// Copyright 2023 Stephen Melnick

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GrimfallGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class FOCI_API UGrimfallGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
protected:
	virtual void OnStart() override;
};
