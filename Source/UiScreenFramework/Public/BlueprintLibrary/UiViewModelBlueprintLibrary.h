// Copyright People Can Fly. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UiViewModelBlueprintLibrary.generated.h"

class UDynamicEntryBox;
class UPanelWidget;
class UPanelSlot;
class UUserWidget;
class UBaseViewModel;

UCLASS()
class UISCREENFRAMEWORK_API UUiViewModelBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Sets a ViewModel manually by class name for a given Widget.
	 * @return True if added successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "UI | ViewModel", Meta=(DefaultToSelf="Widget"))
	static bool SetViewModel(UUserWidget* Widget, UBaseViewModel* ViewModel);

	/*Creates widgets and applies view models then places widgets into specified container*/
	UFUNCTION(BlueprintCallable)
	static TArray<UPanelSlot*> CreateAndAddChildWidgetsForViewModels(const TArray<UBaseViewModel*>& ViewModels,
		UPanelWidget* PanelWidget, const TSubclassOf<UUserWidget> WidgetClassToSpawn,
		const bool bClearContainerFirst = true);

	/*Creates a single widget and applies a view model, places widget into specified container*/
	UFUNCTION(BlueprintCallable)
	static UPanelSlot* CreateAndAddChildWidgetForViewModel(UBaseViewModel* ViewModel,
		UPanelWidget* PanelWidget, const TSubclassOf<UUserWidget> WidgetClassToSpawn,
		const bool bClearContainerFirst = true);

	/**
	 * Fill dynamic entry box with widgets and assign models to those widgets
	 * @param InViewModels - models to assign
	 * @param InBox - widget to populate
	 * @param bInClearContainerFirst - should existing widgets be removed and their models cleared?
	 */
	UFUNCTION(BlueprintCallable)
	static void PopulateDynamicEntryBox(const TArray<UBaseViewModel*>& InViewModels, UDynamicEntryBox* InBox, bool bInClearContainerFirst = true);
};
