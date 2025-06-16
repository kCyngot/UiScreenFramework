// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Structs/MainLayoutWidgetInfo.h"
#include "Structs/UiScreenState.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "UiScreenManager.generated.h"

struct FUiScreenInfo;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnUiScreenChanged, const FGameplayTag /* Previous Screen Tag */, const FGameplayTag /* Current Screen Tag */);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUiScreenChanged_BP, const FGameplayTag, PreviousScreenTag, const FGameplayTag, CurrentScreenTag);

/**
 * @class UUiScreenManager
 * @brief A local player subsystem responsible for managing the lifecycle of UI screens.
 *
 * This manager handles the creation and destruction of the main UI layout,
 * navigation between different UI screens (e.g., main menu, settings, HUD),
 * and state management, including screen history for "back" functionality.
 */
UCLASS(config = Game)
class UISCREENFRAMEWORK_API UUiScreenManager : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	static UUiScreenManager* Get(const AController* Controller);
	static UUiScreenManager* Get(const UObject* WorldContextObject);
	static UUiScreenManager& GetChecked(const UObject* WorldContextObject);

	//~ Begin ULocalPlayerSubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End ULocalPlayerSubsystem interface

	/**
	 * @brief Initializes the manager with the current player controller.
	 * This is typically called when the player controller is set or changed.
	 * @param PlayerController The owning player controller.
	 */
	void InitializeUiScreenManager(APlayerController* PlayerController);

	/**
	 * @brief Changes the active UI screen to the one specified by the tag.
	 * @param ScreenTag The tag identifying the screen to navigate to.
	 * @param InitializeViewModelCallback An optional callback to initialize the screen's view model.
	 */
	void ChangeUiScreen(const FGameplayTag ScreenTag, TFunction<void(UScreenViewModel*)> InitializeViewModelCallback = nullptr);

	/**
	 * @brief Navigates to the previously displayed UI screen.
	 * Does nothing if there is no screen in the history.
	 */
	void GoToThePreviousUiScreen();

	const FUiScreenState& GetCurrentUiScreenData() const { return CurrentScreenState; }
	const FMainLayoutWidgetInfo& GetMainLayoutWidgetInfo() const { return MainLayoutWidgetInfo; }

	FOnUiScreenChanged OnUiScreenChanged;

	/* Triggered whenever the screen is changed by any cause. Supplies info about the previous and new screens. */
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnUiScreenChanged"))
	FOnUiScreenChanged_BP OnUiScreenChanged_BP;

protected:
	void AddMainLayoutToViewport(UMainUiLayoutWidget* LayoutWidget);
	void RemoveMainLayoutFromViewport(UMainUiLayoutWidget& LayoutWidget);

	/**
	 * @brief Creates the main layout widget and stores it.
	 * @param PlayerController The player controller that will own the widget.
	 */
	void CreateMainLayoutWidget(APlayerController* PlayerController);

	void RemoveMainLayoutWidget();
	void DeinitCurrentScreenViewModel();

	/** Information about the currently active main layout widget instance. */
	UPROPERTY(Transient)
	FMainLayoutWidgetInfo MainLayoutWidgetInfo;

	/** The state of the currently displayed UI screen, including history. */
	UPROPERTY(Transient)
	FUiScreenState CurrentScreenState;

	/** The Z-order to use when adding the main layout widget to the viewport. */
	int32 MainLayoutWidgetZOrder = 1000;

private:
	/**
	 * @brief Updates the current screen state and manages the screen history stack.
	 * @param FoundViewInfo Information about the new screen being set.
	 */
	void SetCurrentScreenState(const FUiScreenInfo& FoundViewInfo);

	/**
	 * @brief Creates and initializes a view model for a given screen.
	 * @param InitializeViewModelCallback Optional callback for custom initialization.
	 * @param FoundViewInfo Information about the screen for which to create the view model.
	 */
	void CreateAndInitializeScreenViewModel(TFunction<void(UScreenViewModel*)> InitializeViewModelCallback, const FUiScreenInfo& FoundViewInfo);
	void BroadcastScreenChange(const FGameplayTag PreviousScreenTag, const FGameplayTag CurrentScreenTag) const;
};
