#pragma once
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "ViewModels/ScreenViewModel.h"

#include "UiScreenInfo.generated.h"

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
	TSoftClassPtr<UCommonUserWidget> ScreenClass;

	// Main view model class
	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UScreenViewModel> ScreeViewModelClass;
};