// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "Structs/LayerInfo.h"
#include "Structs/UiScreenInfo.h"
#include "MainUiLayoutWidget.generated.h"
DECLARE_DELEGATE_OneParam(FOnDisplayedWidgetChanged, UCommonActivatableWidget* /* Current Screen Widget */);

class UOverlay;

/**
 * @class UMainUiLayoutWidget
 * @brief Manages the main layout of the UI, organizing different screens into distinct, ordered layers.
 */
UCLASS(Abstract)
class UISCREENFRAMEWORK_API UMainUiLayoutWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	ULayerWidget* GetLayerForScreenInfo(const FUiScreenInfo& UiScreenInfo);
	bool TrySwitchToExistingScreenInLayer(const FUiScreenInfo& UiScreenInfo);
	/**
	 * @brief Sets the content for a specific layer and clears all layers above it.
	 * @param UiScreenInfo The information about the screen to display, including the target layer and widget class.
	 * @param bCleanUpExistingScreens The information whether first all screens should be cleaned up
	 */
	void SetWidgetForLayer(const FUiScreenInfo& UiScreenInfo, const bool bCleanUpExistingScreens);
	UOverlay* GetTooltipLayer() const { return TooltipLayer; }
	void RemoveScreensFromHigherLayer(const FUiScreenInfo& UiScreenInfo);
	UCommonActivatableWidget* GetCurrentScreenWidget();

	FOnDisplayedWidgetChanged OnDisplayedWidgetChangedDelegate;
protected:
	virtual void NativeDestruct() override;

	void OnDisplayedWidgetChanged(UCommonActivatableWidget* CommonActivatableWidget);
	/**
	 * @brief Adds and registers a new layer. Layers should be registered in their Z-order, from bottom to top.
	 * @param LayerTag The gameplay tag identifying the layer (e.g., "UI.Layer.Game").
	 * @param LayerWidget The lazy widget that will host the content for this layer.
	 */
	UFUNCTION(BlueprintCallable, Category = "UI Layout")
	void RegisterLayer(UPARAM(meta = (Categories = "UI.Layer"))
		FGameplayTag LayerTag, ULayerWidget* LayerWidget);

	/**
	 * @brief Adds and registers a tooltip layer.
	 * @param LayerWidget The container for all tootips, which should be the overlay type
	 */
	UFUNCTION(BlueprintCallable, Category = "UI Layout")
	void RegisterTooltipLayer(UOverlay* LayerWidget);

private:
	/**
	 * @brief The registered layers for the primary layout.
	 * The order of elements in this array defines the visual stacking order (Z-order),
	 * where index 0 is the bottom-most layer.
	 */
	UPROPERTY(Transient)
	TArray<FLayerInfo> Layers;

	/**
	 * @brief Cached tooltip layer.
	 */
	UPROPERTY(Transient)
	TObjectPtr<UOverlay> TooltipLayer;

	TWeakObjectPtr<UCommonActivatableWidget> CurrentScreenWidget;
};
