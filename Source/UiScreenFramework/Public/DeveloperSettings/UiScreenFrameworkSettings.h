// Copyright People Can Fly. All Rights Reserved."

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
	float GetTooltipEdgePadding() const { return TooltipEdgePadding; }

private:
	/** The class for the main layout widget that hosts all UI layers. Set in config. */
	UPROPERTY(config, EditAnywhere, Category = "UI")
	TSoftClassPtr<UMainUiLayoutWidget> LayoutWidgetClass;

	/** The data asset containing definitions for all available UI screens. Set in config. */
	UPROPERTY(config, EditAnywhere, Category = "UI")
	TSoftObjectPtr<UUiScreensData> ScreensData;

	/** Minimal distance between edge of the screen and a tooltip edge. */
	UPROPERTY(config, EditAnywhere, Category = "UI")
	float TooltipEdgePadding = 20.f;
};
