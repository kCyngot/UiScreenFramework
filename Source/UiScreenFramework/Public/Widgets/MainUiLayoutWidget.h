// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "Structs/LayerInfo.h"
#include "Structs/UiScreenInfo.h"
#include "MainUiLayoutWidget.generated.h"

class UCommonLazyWidget;
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
	/**
	 * @brief Sets the content for a specific layer and clears all layers above it.
	 * @param UiScreenInfo The information about the screen to display, including the target layer and widget class.
	 */
	void SetWidgetForLayer(const FUiScreenInfo& UiScreenInfo);
	UOverlay* GetTooltipLayer() const { return TooltipLayer; }

protected:
	virtual void NativeDestruct() override;

	/**
	 * @brief Adds and registers a new layer. Layers should be registered in their Z-order, from bottom to top.
	 * @param LayerTag The gameplay tag identifying the layer (e.g., "UI.Layer.Game").
	 * @param LayerWidget The lazy widget that will host the content for this layer.
	 */
	UFUNCTION(BlueprintCallable, Category = "UI Layout")
	void RegisterLayer(UPARAM(meta = (Categories = "UI.Layer"))
		FGameplayTag LayerTag, UCommonLazyWidget* LayerWidget);

	/**
	 * @brief Adds and registers a tooltip layer.
	 * @param LayerWidget The container for all tootips, which should be the overlay type
	 */
	UFUNCTION(BlueprintCallable, Category = "UI Layout")
	void RegisterTooltipLayer(UOverlay* LayerWidget);

	/** Callback for when a lazy widget's loading state changes. */
	void OnLoadingStateChanged(bool bIsLoading);

	/** Callback for when a lazy widget's content has been loaded and set. */
	void OnContentChanged(UUserWidget* UserWidget, FGameplayTag Layer);

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
};
