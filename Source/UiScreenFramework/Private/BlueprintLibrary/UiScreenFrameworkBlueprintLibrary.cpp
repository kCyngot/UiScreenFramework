// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintLibrary/UiScreenFrameworkBlueprintLibrary.h"

#include "Helpers/GeneralHelper.h"
#include "Helpers/UiScreenManagerHelper.h"
#include "Logging/LogUiScreenManager.h"
#include "Subsystems/UiScreenManager.h"
#include "Subsystems/UiScreenTooltipManager.h"

void UUiScreenFrameworkBlueprintLibrary::ChangeUiScreen(const FGameplayTag ScreenTag, const FInitializeViewModelSignature& InitializeViewModelCallback, const UObject* WorldContextObject)
{
	UUiScreenManager& UiScreenManager = UUiScreenManager::GetChecked(WorldContextObject);

	const TFunction<void(UScreenViewModel*)> InitializeViewModelLambda = [InitializeViewModelCallback](UScreenViewModel* ViewModel)
	{
		if (InitializeViewModelCallback.IsBound())
		{
			InitializeViewModelCallback.Execute(ViewModel);
		}
	};

	UE_LOG(LogUiScreenFramework, Log, TEXT("%hs Changing UI screen to: %s"), __FUNCTION__, *ScreenTag.ToString());
	UiScreenManager.ChangeUiScreen(ScreenTag, InitializeViewModelLambda);
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

UUiScreenManager* UUiScreenFrameworkBlueprintLibrary::GetUiScreenManager(const UObject* WorldContextObject)
{
	return GeneralHelper::GetLocalPlayerSubsystem<UUiScreenManager>(WorldContextObject);
}

UUiScreenTooltipManager* UUiScreenFrameworkBlueprintLibrary::GetTooltipSubsystem(const UObject* WorldContextObject)
{
	return GeneralHelper::GetLocalPlayerSubsystem<UUiScreenTooltipManager>(WorldContextObject);
}
