// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Structs/TooltipData.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "UiScreenTooltipManager.generated.h"


UCLASS()
class UISCREENFRAMEWORK_API UUiScreenTooltipManager : public ULocalPlayerSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;

	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FTooltipManager, STATGROUP_Tickables);
	}

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	FTooltipData* RegisterTooltip(const FTooltipData& TooltipData);

	UFUNCTION(BlueprintCallable)
	void DisplayTooltip(const FTooltipData& TooltipData);

	UFUNCTION(BlueprintCallable)
	void HideCurrentTooltip();

private:
	void ClearAllTooltips();
	void OnUiScreenChanged(const FGameplayTag PreviousScreenTag, const FGameplayTag CurrentScreenTag);
	void SetBindings(bool bShouldBind);
	void SetTooltipPosition(const TObjectPtr<UUserWidget>& TooltipWidget, FVector2D TooltipPosition) const;

	UPROPERTY(Transient)
	TMap<FGameplayTag, FTooltipData> Tooltips;

	FGameplayTag CurrentTooltipTag;
};
