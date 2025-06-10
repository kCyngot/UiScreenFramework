// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UiScreenFrameworkBlueprintLibrary.generated.h"

class UUiScreenManager;
DECLARE_DYNAMIC_DELEGATE_OneParam(FInitializeViewModelSignature, UScreenViewModel*, ViewModel);

class UScreenViewModel;

UCLASS()
class UISCREENFRAMEWORK_API UUiScreenFrameworkBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Ui Screen Framework", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "InitializeViewModelCallback"))
	static void ChangeUiScreen(UPARAM(meta = (Categories = "UI.Screen"))
		const FGameplayTag ScreenTag, const FInitializeViewModelSignature& InitializeViewModelCallback, const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Ui Screen Framework", meta = (WorldContext = "WorldContextObject"))
	static void GoToThePreviousUiScreen(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "Ui Screen Framework", meta = (WorldContext = "WorldContextObject"))
	static FGameplayTag GetCurrentUiScreenId(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "Ui Screen Framework", meta = (WorldContext = "WorldContextObject"))
	static UUiScreenManager* GetUiScreenManager(const UObject* WorldContextObject);
};
