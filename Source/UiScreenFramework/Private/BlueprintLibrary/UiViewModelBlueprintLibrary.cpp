// Copyright People Can Fly. All Rights Reserved.


#include "BlueprintLibrary/UiViewModelBlueprintLibrary.h"

#include "Components/DynamicEntryBox.h"
#include "Components/PanelWidget.h"
#include "Helpers/ViewModelHelper.h"
#include "Logging/LogUiScreenManager.h"
#include "Templates/SubclassOf.h"
#include "View/MVVMView.h"
#include "View/MVVMViewClass.h"
#include "ViewModels/BaseViewModel.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(UiViewModelBlueprintLibrary)

bool UUiViewModelBlueprintLibrary::SetViewModel(UUserWidget* Widget, UBaseViewModel* ViewModel)
{
	return ViewModelHelper::SetViewModel(Widget, ViewModel);
}

TArray<UPanelSlot*> UUiViewModelBlueprintLibrary::CreateAndAddChildWidgetsForViewModels(const TArray<UBaseViewModel*>& ViewModels, UPanelWidget* PanelWidget,
	const TSubclassOf<UUserWidget> WidgetClassToSpawn, const bool bClearContainerFirst)
{
	if (!IsValid(PanelWidget))
	{
		return {};
	}

	if (bClearContainerFirst)
	{
		PanelWidget->ClearChildren();
	}

	if (!WidgetClassToSpawn)
	{
		UE_LOG(LogUiScreenFramework, Error, TEXT("%hs: WidgetClassToSpawn is not given, panel widget owner = %s"),
			__FUNCTION__, *GetPathNameSafe(PanelWidget->GetOuter()));
		return {};
	}

	TArray<UPanelSlot*> NewlyFilledSlots;
	for (UBaseViewModel* ViewModel : ViewModels)
	{
		if (!ensure(IsValid(ViewModel)))
		{
			continue;
		}

		UUserWidget* NewWidget = CreateWidget(PanelWidget->GetOwningPlayer(), WidgetClassToSpawn);
		UPanelSlot* NewSlot = PanelWidget->AddChild(NewWidget);
		NewlyFilledSlots.Add(NewSlot);
		ViewModelHelper::SetViewModel(NewWidget, ViewModel);
	}

	return NewlyFilledSlots;
}

UPanelSlot* UUiViewModelBlueprintLibrary::CreateAndAddChildWidgetForViewModel(UBaseViewModel* ViewModel, UPanelWidget* PanelWidget, const TSubclassOf<UUserWidget> WidgetClassToSpawn,
	const bool bClearContainerFirst)
{
	const TArray<UPanelSlot*>& NewlyFilledSlotArray = CreateAndAddChildWidgetsForViewModels({ViewModel}, PanelWidget, WidgetClassToSpawn, bClearContainerFirst);
	if (NewlyFilledSlotArray.IsEmpty())
	{
		return nullptr;
	}

	return NewlyFilledSlotArray[0];
}

void UUiViewModelBlueprintLibrary::PopulateDynamicEntryBox(const TArray<UBaseViewModel*>& InViewModels, UDynamicEntryBox* InBox, bool bInClearContainerFirst)
{
	if (!InBox)
	{
		UE_LOG(LogUiScreenFramework, Error, TEXT("%hs - InBox is invalid!"), __FUNCTION__);
		return;
	}
	
	if (bInClearContainerFirst)
	{
		InBox->Reset<UUserWidget>([](const UUserWidget& Widget)
		{
			const UMVVMView* Extension = ViewModelHelper::GetMvvmViewExtensionFromWidget(&Widget);
			if (!Extension)
			{
				return;
			}
			
			for (const FMVVMViewClass_Source& Source : Extension->GetViewClass()->GetSources())
			{
				if (UBaseViewModel* BaseViewModel = Cast<UBaseViewModel>(Extension->GetViewModel(Source.GetName()).GetObject()))
				{
					BaseViewModel->Deinit();
				}
			}
		});
	}
	
	for (UBaseViewModel* ViewModel : InViewModels)
	{
		ViewModelHelper::SetViewModel(InBox->CreateEntry(), ViewModel);
	}
}
