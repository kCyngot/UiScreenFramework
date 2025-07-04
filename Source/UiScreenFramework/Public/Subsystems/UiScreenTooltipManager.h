// Copyright People Can Fly. All Rights Reserved."

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Structs/TooltipData.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "UiScreenTooltipManager.generated.h"

/**
 * @class UUiScreenTooltipManager
 * @brief Manages the lifecycle of UI tooltips for a local player.
 *
 * This subsystem is responsible for creating, displaying, positioning, and destroying tooltip widgets.
 * It implements FTickableGameObject to update the tooltip's position every frame, ensuring it
 * stays anchored correctly. It also listens for UI screen changes to automatically clear tooltips.
 */
UCLASS()
class UISCREENFRAMEWORK_API UUiScreenTooltipManager : public ULocalPlayerSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	//~ Begin FTickableGameObject interface
	virtual void Tick(float DeltaTime) override;

	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FTooltipManager, STATGROUP_Tickables);
	}
	//~ End FTickableGameObject interface

	//~ Begin ULocalPlayerSubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End ULocalPlayerSubsystem interface

	/**
	 * @brief Pre-creates and registers a tooltip widget instance from the provided data.
	 * The tooltip is added to a central overlay but remains hidden until displayed.
	 * @param TooltipData The data required to create the tooltip, including its class and view model.
	 * @return A pointer to the stored FTooltipData if successful, otherwise nullptr.
	 */
	FTooltipData* RegisterTooltip(const FTooltipData& TooltipData);

	/**
	 * @brief Displays a tooltip based on the request data.
	 * If the tooltip is not already registered, it will be registered first.
	 * Then, its visibility is enabled, and its content and position are updated.
	 * @param TooltipRequestData The data specifying which tooltip to show and where.
	 */
	void DisplayTooltip(const FTooltipData& TooltipRequestData);

	/**
	 * @brief Hides the tooltip that is currently being displayed.
	 */
	void HideCurrentTooltip();

private:
	/**
	 * @brief Clears and destroys all registered tooltip widgets.
	 * Typically called when the UI screen changes to prevent orphaned tooltips.
	 */
	void ClearAllTooltips();

	/**
	 * @brief Callback function that is executed when the main UI screen changes.
	 * @param PreviousScreenTag The gameplay tag of the screen being left.
	 * @param CurrentScreenTag The gameplay tag of the screen being entered.
	 */
	void OnUiScreenChanged(const FGameplayTag PreviousScreenTag, const FGameplayTag CurrentScreenTag);

	/**
	 * @brief Binds or unbinds the manager's event handlers to other subsystems (e.g., UUiScreenManager).
	 * @param bShouldBind True to bind, false to unbind.
	 */
	void SetBindings(bool bShouldBind);

	/**
	 * @brief Sets the final screen position of the tooltip widget within its overlay slot.
	 * @param TooltipWidget The tooltip widget to position.
	 * @param TooltipPosition The calculated 2D position on the screen.
	 */
	void SetTooltipPosition(const TObjectPtr<UUserWidget>& TooltipWidget, FVector2D TooltipPosition) const;

	/**
	 * @brief Sets or updates the view model for a given tooltip widget.
	 * @param TooltipWidget The widget whose view model should be set.
	 * @param TooltipViewModel The view model instance to assign to the widget.
	 */
	static void SetViewModelForTooltipWidget(const UUserWidget* TooltipWidget, UBaseViewModel* TooltipViewModel);

	/** A map of all registered tooltips, keyed by a unique name derived from their widget class. */
	UPROPERTY(Transient)
	TMap<FName, FTooltipData> Tooltips;

	/** The name/key of the tooltip that is currently visible. */
	FName CurrentTooltip;
};