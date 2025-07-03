// Copyright People Can Fly. All Rights Reserved."

#pragma once
#include "GameplayTagContainer.h"
#include "DeveloperSettings/UiScreenFrameworkSettings.h"

class UScreenViewModel;

namespace UiScreenManagerHelper
{
	FGameplayTag GetCurrentScreenTag(const UObject* WorldContextObject);
	TObjectPtr<UScreenViewModel> GetCurrentScreenViewModel(const UObject* WorldContextObject);
	const UUiScreenFrameworkSettings& GetUiScreenFrameworkSettings();
}
