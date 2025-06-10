#pragma once
#include "GameplayTagContainer.h"

#include "UiScreenState.generated.h"

class UCommonUserWidget;
class UCommonActivatableWidget;
class UScreenViewModel;

USTRUCT(BlueprintType)
struct FUiScreenState
{
	GENERATED_BODY()

	// Screen Id
	UPROPERTY(BlueprintReadOnly, meta = (Categories = "UI.Screen"))
	FGameplayTag ScreenId = FGameplayTag::EmptyTag;

	// Layer Id
	UPROPERTY(BlueprintReadOnly, meta = (Categories = "UI.Layer"))
	FGameplayTag LayerId = FGameplayTag::EmptyTag;

	// Screen widget
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UCommonUserWidget> ScreenWidget;

	// Main view model
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UScreenViewModel> ScreeViewModel;

	// Array of previous screens
	UPROPERTY(BlueprintReadOnly)
	TArray<FGameplayTag> PreviousScreens;

	FUiScreenState()
	{
	}

	FUiScreenState(const FGameplayTag InScreenId, const FGameplayTag InLayerId, UCommonUserWidget* InScreenWidget, UScreenViewModel* InScreeViewModel)
		: ScreenId(InScreenId)
		, LayerId(InLayerId)
		, ScreenWidget(InScreenWidget)
		, ScreeViewModel(InScreeViewModel)
	{
	}

	bool operator==(const FGameplayTag OtherScreenId) const { return ScreenId == OtherScreenId; }
};