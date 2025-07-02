// Fill out your copyright notice in the Description page of Project Settings.


#include "Resolvers/ScreenViewModelResolver.h"

#include "Helpers/UiScreenManagerHelper.h"
#include "Logging/LogUiScreenManager.h"
#include "ViewModels/ScreenViewModel.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(ScreenViewModelResolver)

UObject* UScreenViewModelResolver::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const
{
	if (!ExpectedType)
	{
		UE_LOG(LogUiScreenFramework, Error, TEXT("CreateInstance failed: ExpectedType is null."));
		return nullptr;
	}

	if (!UserWidget)
	{
		UE_LOG(LogUiScreenFramework, Warning, TEXT("CreateInstance failed for type '%s': UserWidget is null."), *ExpectedType->GetName());
		return nullptr;
	}

	TObjectPtr<UScreenViewModel> CurrentScreenViewModel = UiScreenManagerHelper::GetCurrentScreenViewModel(UserWidget);

	if (!CurrentScreenViewModel)
	{
		UE_LOG(LogUiScreenFramework, Error, TEXT("No CurrentScreenViewModel found for UserWidget '%s'."), *UserWidget->GetName());
		return nullptr;
	}

	if (CurrentScreenViewModel->IsA(ExpectedType))
	{
		UE_LOG(LogUiScreenFramework, Log, TEXT("Found matching ScreenViewModel of type '%s' for UserWidget '%s'."), *ExpectedType->GetName(), *UserWidget->GetName());
		return CurrentScreenViewModel;
	}

	UE_LOG(LogUiScreenFramework, Warning, TEXT("Type mismatch: CurrentScreenViewModel is '%s' but MVVM View expected '%s'."),
		*CurrentScreenViewModel->GetClass()->GetName(),
		*ExpectedType->GetName());
	return nullptr;
}
