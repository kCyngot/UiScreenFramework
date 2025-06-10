// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/UiScreensData.h"
#include "Engine/DeveloperSettings.h"
#include "Widgets/MainUiLayoutWidget.h"
#include "UiScreenFrameworkSettings.generated.h"

UCLASS(config = Game, defaultconfig)
class UISCREENFRAMEWORK_API UUiScreenFrameworkSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	TSubclassOf<UMainUiLayoutWidget> GetLayoutWidgetClass() const { return LayoutWidgetClass.LoadSynchronous(); }
	UUiScreensData* GetViewsData() const { return ScreensData.LoadSynchronous(); }

private:
	/** The class for the main layout widget that hosts all UI layers. Set in config. */
	UPROPERTY(config, EditAnywhere, Category = "UI")
	TSoftClassPtr<UMainUiLayoutWidget> LayoutWidgetClass;

	/** The data asset containing definitions for all available UI screens. Set in config. */
	UPROPERTY(config, EditAnywhere, Category = "UI")
	TSoftObjectPtr<UUiScreensData> ScreensData;
};
