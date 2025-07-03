// Copyright People Can Fly. All Rights Reserved."

#pragma once
#include "GameplayTagContainer.h"

#include "ScreenInitialData.generated.h"


class UCommonActivatableWidget;
class UScreenViewModel;

USTRUCT(BlueprintType)
struct FScreenInitialData
{
	GENERATED_BODY()

	/* The unique tag that identifies this screen. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "UI.Screen"))
	FGameplayTag ScreenTag;

	/* Indicates whether we need to clean up all screens first (fresh start) */
	bool bCleanUpExistingScreens = false;

	/* Function that initializes the screen view model. */
	TFunction<void(UScreenViewModel*)> InitializeViewModelCallback;

	/* Function that initializes the screen widget. */
	TFunction<void(UCommonActivatableWidget*)> InitializeScreenWidgetCallback;

	FScreenInitialData() = default;

	FScreenInitialData(const FGameplayTag InScreenId, const bool bInCleanUpExistingScreens, const TFunction<void(UScreenViewModel*)>& InInitializeViewModelCallback,
		const TFunction<void(UCommonActivatableWidget*)>& InInitializeScreenWidgetCallback)
		: ScreenTag(InScreenId)
		  , bCleanUpExistingScreens(bInCleanUpExistingScreens)
		  , InitializeViewModelCallback(InInitializeViewModelCallback)
		  , InitializeScreenWidgetCallback(InInitializeScreenWidgetCallback)
	{
	}

	FScreenInitialData(const FGameplayTag InScreenId)
		: ScreenTag(InScreenId)
	{
	}
};
