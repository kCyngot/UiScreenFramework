// Copyright People Can Fly. All Rights Reserved."

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "Structs/LayerInfo.h"
#include "Structs/UiScreenInfo.h"
#include "MainUiLayoutWidget.generated.h"

class UOverlay;

/**
 * @brief Delegate that is broadcast whenever the currently displayed widget changes.
 * @param CurrentScreenWidget A pointer to the newly displayed activatable widget.
 */
DECLARE_DELEGATE_OneParam(FOnDisplayedWidgetChanged, UCommonActivatableWidget* /* Current Screen Widget */);


/**
 * @class UMainUiLayoutWidget
 * @brief Manages the main layout of the UI, organizing different screens into distinct, ordered layers.
 * This widget acts as the root container for all UI screens in the game. It uses a layer-based system
 * to manage different types of UI, such as the game HUD, menus, and modal dialogs.
 */
UCLASS(Abstract)
class UISCREENFRAMEWORK_API UMainUiLayoutWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * @brief Gets the layer widget associated with the given screen information.
	 * @param UiScreenInfo The screen information containing the target layer ID.
	 * @return The ULayerWidget instance for the specified layer, or nullptr if not found.
	 */
	ULayerWidget* GetLayerForScreenInfo(const FUiScreenInfo& UiScreenInfo);

	/**
	 * @brief Checks if a widget of the same class as the one in UiScreenInfo already exists in the target layer and, if so, switches to it.
	 * @param UiScreenInfo The information about the screen to potentially switch to.
	 * @return True if an existing widget was found and switched to, false otherwise.
	 */
	bool TrySwitchToExistingScreenInLayer(const FUiScreenInfo& UiScreenInfo);

	/**
	 * @brief Sets the content for a specific layer.
	 * If bCleanUpExistingScreens is true, it clears all layers first. Otherwise, it clears layers above the target layer.
	 * @param UiScreenInfo The information about the screen to display, including the target layer and widget class.
	 * @param bCleanUpExistingScreens If true, all existing screens on all layers are removed before adding the new one.
	 */
	void SetWidgetForLayer(const FUiScreenInfo& UiScreenInfo, const bool bCleanUpExistingScreens);

	/**
	 * @brief Gets the dedicated overlay widget for displaying tooltips.
	 * @return A pointer to the tooltip layer overlay.
	 */
	UOverlay* GetTooltipLayer() const { return TooltipLayer; }

	/**
	 * @brief Removes all screens from layers that are stacked on top of the layer specified in UiScreenInfo.
	 * @param UiScreenInfo The information about the screen whose layer will be the new top-most persistent layer.
	 */
	void RemoveScreensFromHigherLayer(const FUiScreenInfo& UiScreenInfo);
	
	/**
	 * @brief Gets the currently active and visible screen widget.
	 * @return A pointer to the current UCommonActivatableWidget, or nullptr if none is active.
	 */
	UCommonActivatableWidget* GetCurrentScreenWidget();

	/** Delegate broadcasted when the displayed widget changes, used by UiScreenManager. */
	FOnDisplayedWidgetChanged OnDisplayedWidgetChangedDelegate;

protected:
	/**
	 * @brief Overridden from UUserWidget. Cleans up bindings when the widget is destroyed.
	 */
	virtual void NativeDestruct() override;

	/**
	 * @brief Handles the OnDisplayedWidgetChanged event from a ULayerWidget.
	 * Caches the current screen widget and broadcasts the main delegate.
	 * @param CommonActivatableWidget The new widget that is being displayed.
	 */
	void OnDisplayedWidgetChanged(UCommonActivatableWidget* CommonActivatableWidget);
	
	/**
	 * @brief Adds and registers a new UI layer. Layers should be registered in their Z-order, from bottom to top.
	 * This should be called from the Blueprint implementation of this class.
	 * @param LayerTag The gameplay tag identifying the layer (e.g., "UI.Layer.Game").
	 * @param LayerWidget The ULayerWidget instance that will host the content for this layer.
	 */
	UFUNCTION(BlueprintCallable, Category = "UI Layout")
	void RegisterLayer(UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag LayerTag, ULayerWidget* LayerWidget);

	/**
	 * @brief Registers the dedicated overlay for tooltips.
	 * @param LayerWidget The container (UOverlay) for all tooltips.
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
	 * @brief Cached tooltip layer widget. This is a simple UOverlay where tooltips are rendered.
	 */
	UPROPERTY(Transient)
	TObjectPtr<UOverlay> TooltipLayer;

	/** A weak pointer to the currently active screen widget. */
	TWeakObjectPtr<UCommonActivatableWidget> CurrentScreenWidget;
};