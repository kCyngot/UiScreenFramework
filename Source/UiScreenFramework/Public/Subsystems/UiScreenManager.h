#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/StreamableManager.h"
#include "Structs/MainLayoutWidgetInfo.h"
#include "Structs/ScreenInitialData.h"
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
 * creating and managing screen view models,
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
	 * This creates the main layout widget and prepares the manager for use.
	 * @param PlayerController The owning player controller.
	 */
	void InitializeUiScreenManager(APlayerController* PlayerController);

	/**
	 * @brief Callback executed after a screen's widget class has been asynchronously loaded.
	 * Proceeds with displaying the screen.
	 * @param ScreenInitialData The initial data for the screen to be displayed.
	 */
	void OnScreenWidgetClassLoaded(FScreenInitialData ScreenInitialData);

	/**
	 * @brief Changes the active UI screen to the one specified by the tag inside Initial data struct.
	 * This is the primary method for navigating between UI screens.
	 * @param ScreenInitialData Data required to initialize the new screen.
	 */
	void ChangeUiScreen(FScreenInitialData ScreenInitialData);

	/**
	 * @brief Navigates to the previously displayed UI screen based on the screen history.
	 * Does nothing if there is no screen in the history.
	 */
	void GoToThePreviousUiScreen();

	/**
	 * @brief Retrieves the configuration info for a given screen tag from the UI Screens Data asset.
	 * @param ScreenTag The tag of the screen to find.
	 * @return A pointer to the screen info struct, or nullptr if not found.
	 */
	FUiScreenInfo* GetUiScreenInfo(const FGameplayTag ScreenTag);

	/** Gets the current UI screen state, including the active screen tag and history. */
	const FUiScreenState& GetCurrentUiScreenData() const { return CurrentScreenState; }
	/** Gets information about the main layout widget, such as the widget instance and player owner. */
	const FMainLayoutWidgetInfo& GetMainLayoutWidgetInfo() const { return MainLayoutWidgetInfo; }

	/**
	 * @brief Gets the cached view model for a specific screen.
	 * @param ScreenTag The tag of the screen whose view model is requested.
	 * @return The screen view model, or nullptr if it doesn't exist.
	 */
	UScreenViewModel* GetScreenViewModel(const FGameplayTag ScreenTag);

	/** Gets the currently active and displayed screen widget instance. */
	UCommonActivatableWidget* GetScreenWidget() const;

	/**
	 * @brief Executes the initialization callback for a newly created screen widget.
	 * @param ScreenWidget The screen widget instance to be initialized.
	 */
	void CallInitializeScreenWidgetCallback(UCommonActivatableWidget* ScreenWidget);

	/** Delegate broadcasted when the UI screen changes. */
	FOnUiScreenChanged OnUiScreenChanged;

	/* Blueprint-assignable delegate triggered whenever the screen is changed. */
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnUiScreenChanged"))
	FOnUiScreenChanged_BP OnUiScreenChanged_BP;

protected:
	/** Adds the main layout widget to the player's viewport. */
	void AddMainLayoutToViewport(UMainUiLayoutWidget* LayoutWidget);
	/** Removes the main layout widget from the player's viewport. */
	void RemoveMainLayoutFromViewport(UMainUiLayoutWidget& LayoutWidget);

	/**
	 * @brief Creates the main layout widget that will host all other UI elements.
	 * @param PlayerController The player controller that will own the widget.
	 */
	void CreateMainLayoutWidget(APlayerController* PlayerController);

	/** Removes and cleans up the main layout widget. */
	void RemoveMainLayoutWidget();

	/** Deinitializes and cleans up a given screen view model. */
	void DeinitScreenViewModel(UScreenViewModel* ScreenViewModel);

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
	 * @param ScreenInitialData The initial data for the new screen.
	 * @param FoundViewInfo Information about the new screen being set.
	 */
	void SetCurrentScreenState(const FScreenInitialData& ScreenInitialData, const FUiScreenInfo& FoundViewInfo);

	/**
	 * @brief Creates a new view model for a screen or reuses an existing one.
	 * @param ScreenInitialData The initial data for the screen.
	 * @param FoundViewInfo Information about the screen for which to create the view model.
	 */
	void CreateOrReuseScreenViewModel(const FScreenInitialData& ScreenInitialData, const FUiScreenInfo& FoundViewInfo);

	/** Cleans up all cached screen view models. */
	void CleanupAllScreenViewModels();

	/** Broadcasts the OnUiScreenChanged and OnUiScreenChanged_BP delegates. */
	void BroadcastScreenChange(const FGameplayTag PreviousScreenTag, const FGameplayTag CurrentScreenTag) const;

	/** Handle for asynchronous asset streaming. */
	TSharedPtr<FStreamableHandle> StreamingHandle;

	/** Queue for holding screen change requests that arrive while another is being processed. */
	TQueue<FScreenInitialData> ScreensToDisplayQueue;

	/** Callback function to initialize the screen widget after it's created. */
	TFunction<void(UCommonActivatableWidget*)> InitializeScreenWidgetCallback;

	/** A map of all active screen view models, keyed by their screen tag. */
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UScreenViewModel>> ScreenViewModelsMap;
};
