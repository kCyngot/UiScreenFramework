#pragma once

#include "GameplayTagContainer.h"

#include "UiScreenInfo.generated.h"

class UCommonActivatableWidget;
class UScreenViewModel;

USTRUCT(BlueprintType)
struct FUiScreenInfo
{
	GENERATED_BODY()

	// Screen Id
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "UI.Screen"))
	FGameplayTag ScreenId = FGameplayTag::EmptyTag;

	// Layer Id
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "UI.Layer"))
	FGameplayTag LayerId = FGameplayTag::EmptyTag;

	// Screen widget class
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftClassPtr<UCommonActivatableWidget> ScreenClass;

	// Main view model class
	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UScreenViewModel> ScreenViewModelClass;
};
