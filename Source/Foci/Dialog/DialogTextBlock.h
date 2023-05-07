// Copyright 2023 Stephen Melnick

#pragma once

#include "CoreMinimal.h"
#include "Components/TextBlock.h"

#include "Tickable.h"

#include "DialogTextBlock.generated.h"

/**
 * 
 */
UCLASS()
class FOCI_API UDialogTextBlock : public UTextBlock, public FTickableGameObject
{
	GENERATED_BODY()


public:
	virtual void SetText(FText InText) override;

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void BindViewModel(class UDialogViewModel* DialogViewModel);

	// Try to skip the animation
	// returns true if this causes the animation to be skipped
	UFUNCTION(BlueprintCallable)
	bool SkipAnimation();

	UFUNCTION(BlueprintCallable)
	void SetDialog(FText Dialog);

	virtual ETickableTickType GetTickableTickType() const override;

	virtual TStatId GetStatId() const override;

	virtual bool IsTickableWhenPaused() const override;

	virtual bool IsTickableInEditor() const override;

private:
	TWeakObjectPtr<class UDialogViewModel> ViewModel;

	bool bAnimationPlaying = false;
	
	float TextCounter = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Animation, meta = (AllowPrivateAccess = "true"))
	float CharactersPerSecond = 1.0f;

	int CurrentTextIndex = 0;

	FString TextString = "Test string";

	FString DisplayText;
};
