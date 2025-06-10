#include "Helpers/UiScreenManagerHelper.h"

#include "Subsystems/UiScreenManager.h"

FGameplayTag UiScreenManagerHelper::GetCurrentScreenTag(const UObject* WorldContextObject)
{
	const UUiScreenManager& UiScreenManager = UUiScreenManager::GetChecked(WorldContextObject);

	return UiScreenManager.GetCurrentUiScreenData().ScreenId;
}

TObjectPtr<UScreenViewModel> UiScreenManagerHelper::GetCurrentScreenViewModel(const UObject* WorldContextObject)
{
	const UUiScreenManager& UiScreenManager = UUiScreenManager::GetChecked(WorldContextObject);

	return UiScreenManager.GetCurrentUiScreenData().ScreeViewModel;
}

const UUiScreenFrameworkSettings& UiScreenManagerHelper::GetUiScreenFrameworkSettings()
{
	const UUiScreenFrameworkSettings* UiScreenFrameworkSettings = GetDefault<UUiScreenFrameworkSettings>();
	checkf(UiScreenFrameworkSettings, TEXT("UiScreenFrameworkSettings was invalid"))

	return *UiScreenFrameworkSettings;
}
