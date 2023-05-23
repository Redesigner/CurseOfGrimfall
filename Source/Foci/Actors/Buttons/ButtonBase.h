#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ButtonBase.generated.h"

UCLASS()
class FOCI_API AButtonBase : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Button, meta = (AllowPrivateAccess = true))
	bool bButtonPressed = false;
	
public:
	AButtonBase();

	void Trigger(AActor* Source);

	UFUNCTION(BlueprintImplementableEvent)
	void OnTriggered(AActor* Source);


	void Release(AActor* Source);

	UFUNCTION(BlueprintImplementableEvent)
	void OnReleased(AActor* Source);


	bool GetButtonPressed() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = Button, meta = (AllowPrivateAccess = true))
	TWeakObjectPtr<AActor> Target;

};
