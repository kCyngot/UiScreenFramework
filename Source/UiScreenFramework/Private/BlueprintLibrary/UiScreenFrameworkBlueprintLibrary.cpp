// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintLibrary/UiScreenFrameworkBlueprintLibrary.h"

#include "Helpers/GeneralHelper.h"
#include "Helpers/UiScreenManagerHelper.h"
#include "Logging/LogUiScreenManager.h"
#include "Subsystems/UiScreenManager.h"
#include "Subsystems/UiScreenTooltipManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UiScreenFrameworkBlueprintLibrary)

void UUiScreenFrameworkBlueprintLibrary::ChangeUiScreen(UPARAM(meta = (Categories = "UI.Screen"))
	const FGameplayTag ScreenTag, const bool bCleanUpExistingScreens, const FInitializeViewModelSignature& InitializeViewModelCallback,
	const FInitializeScreenWidgetSignature& InitializeScreenWidgetCallback, const UObject* WorldContextObject)
{
	UUiScreenManager& UiScreenManager = UUiScreenManager::GetChecked(WorldContextObject);

	const TFunction<void(UScreenViewModel*)> InitializeViewModelLambda = [InitializeViewModelCallback](UScreenViewModel* ViewModel)
	{
		if (InitializeViewModelCallback.IsBound())
		{
			InitializeViewModelCallback.Execute(ViewModel);
		}
	};

	const TFunction<void(UCommonActivatableWidget*)> InitializeScreenWidgetLambda = [InitializeScreenWidgetCallback](UCommonActivatableWidget* ScreenWidget)
	{
		if (InitializeScreenWidgetCallback.IsBound())
		{
			InitializeScreenWidgetCallback.Execute(ScreenWidget);
		}
	};

	UE_LOG(LogUiScreenFramework, Log, TEXT("%hs Changing UI screen to: %s"), __FUNCTION__, *ScreenTag.ToString());
	UiScreenManager.ChangeUiScreen(FScreenInitialData(ScreenTag, bCleanUpExistingScreens, InitializeViewModelLambda, InitializeScreenWidgetLambda));
}

void UUiScreenFrameworkBlueprintLibrary::GoToThePreviousUiScreen(const UObject* WorldContextObject)
{
	UUiScreenManager& UiScreenManager = UUiScreenManager::GetChecked(WorldContextObject);
	UiScreenManager.GoToThePreviousUiScreen();
}

FGameplayTag UUiScreenFrameworkBlueprintLibrary::GetCurrentUiScreenId(const UObject* WorldContextObject)
{
	const UUiScreenManager& UiScreenManager = UUiScreenManager::GetChecked(WorldContextObject);
	return UiScreenManager.GetCurrentUiScreenData().ScreenId;
}

UCommonActivatableWidget* UUiScreenFrameworkBlueprintLibrary::GetCurrentUiScreenWidget(const UObject* WorldContextObject)
{
	const UUiScreenManager& UiScreenManager = UUiScreenManager::GetChecked(WorldContextObject);
	return UiScreenManager.GetScreenWidget();
}

UUiScreenManager* UUiScreenFrameworkBlueprintLibrary::GetUiScreenManager(const UObject* WorldContextObject)
{
	return GeneralHelper::GetLocalPlayerSubsystem<UUiScreenManager>(WorldContextObject);
}

UUiScreenTooltipManager* UUiScreenFrameworkBlueprintLibrary::GetTooltipSubsystem(const UObject* WorldContextObject)
{
	return GeneralHelper::GetLocalPlayerSubsystem<UUiScreenTooltipManager>(WorldContextObject);
}
