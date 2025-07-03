// Copyright People Can Fly. All Rights Reserved."

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UiScreenFrameworkBlueprintLibrary.generated.h"

class UCommonActivatableWidget;
class UUiScreenTooltipManager;
class UUiScreenManager;

DECLARE_DYNAMIC_DELEGATE_OneParam(FInitializeViewModelSignature, UScreenViewModel*, ViewModel);

DECLARE_DYNAMIC_DELEGATE_OneParam(FInitializeScreenWidgetSignature, UCommonActivatableWidget*, ScreenWidget);

class UScreenViewModel;

UCLASS()
class UISCREENFRAMEWORK_API UUiScreenFrameworkBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Ui Screen Framework", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "InitializeViewModelCallback, InitializeScreenWidgetCallback"))
	static void ChangeUiScreen(UPARAM(meta = (Categories = "UI.Screen"))
		const FGameplayTag ScreenTag, const bool bCleanUpExistingScreens, const FInitializeViewModelSignature& InitializeViewModelCallback,
		const FInitializeScreenWidgetSignature& InitializeScreenWidgetCallback, const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Ui Screen Framework", meta = (WorldContext = "WorldContextObject"))
	static void GoToThePreviousUiScreen(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "Ui Screen Framework", meta = (WorldContext = "WorldContextObject"))
	static FGameplayTag GetCurrentUiScreenId(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "Ui Screen Framework", meta = (WorldContext = "WorldContextObject"))
	static UCommonActivatableWidget* GetCurrentUiScreenWidget(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "Ui Screen Framework", meta = (WorldContext = "WorldContextObject"))
	static UUiScreenManager* GetUiScreenManager(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "Ui Screen Framework", meta = (WorldContext = "WorldContextObject"))
	static UUiScreenTooltipManager* GetTooltipSubsystem(const UObject* WorldContextObject);
};
