#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "Foci/Dialog/DialogRequest.h"
#include "Foci/Dialog/DialogResponse.h"

#include "DialogComponent.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class FOCI_API UDialogComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void RequestDialog(class AFociCharacter* RequestSource, FDialogRequest Request);

private:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = true))
	TSoftObjectPtr<class UDataTable> DialogTable;

	FDialogResponse GetDialogResponse(FDialogRequest& Request) const;
};