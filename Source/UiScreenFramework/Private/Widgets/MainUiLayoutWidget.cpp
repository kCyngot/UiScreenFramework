// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/MainUiLayoutWidget.h"

#include "CommonActivatableWidget.h"
#include "Logging/LogUiScreenManager.h"
#include "Subsystems/UiScreenManager.h"
#include "Widgets/LayerWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MainUiLayoutWidget)

ULayerWidget* UMainUiLayoutWidget::GetLayerForScreenInfo(const FUiScreenInfo& UiScreenInfo)
{
	FGameplayTag LayerId = UiScreenInfo.LayerId;
	FLayerInfo* TargetLayerInfo = Layers.FindByPredicate(
		[LayerId](const FLayerInfo& Info)
		{
			return Info.LayerId == LayerId;
		});

	if (!TargetLayerInfo)
	{
		UE_LOG(LogUiScreenFramework, Error, TEXT("%hs: Cannot find layer info for layer %s."), __FUNCTION__, *UiScreenInfo.LayerId.ToString());
		return nullptr;
	}

	ULayerWidget* FoundLayer = TargetLayerInfo->LayerWidget;

	if (!IsValid(FoundLayer))
	{
		UE_LOG(LogUiScreenFramework, Error, TEXT("%hs: Layer widget is nullptr for layer %s."), __FUNCTION__, *UiScreenInfo.LayerId.ToString());
		return nullptr;
	}

	return FoundLayer;
}

bool UMainUiLayoutWidget::TrySwitchToExistingScreenInLayer(const FUiScreenInfo& UiScreenInfo)
{
	ULayerWidget* CurrentLayer = GetLayerForScreenInfo(UiScreenInfo);
	if (!CurrentLayer)
	{
		return false;
	}

	TArray<UCommonActivatableWidget*> ActiveWidgetList = CurrentLayer->GetWidgetList();
	FSoftObjectPath StreamingObjectPath = UiScreenInfo.ScreenClass.ToSoftObjectPath();
	const int32 FoundIndex = ActiveWidgetList.IndexOfByPredicate([StreamingObjectPath](UCommonActivatableWidget* Widget)
	{
		if (IsValid(Widget))
		{
			const FSoftObjectPath WidgetClassPath = Widget->GetClass()->GetPathName();

			return WidgetClassPath == StreamingObjectPath;
		}

		return false;
	});

	if (ActiveWidgetList.IsValidIndex(FoundIndex))
	{
		CurrentLayer->SetSwitcherIndex(FoundIndex + 1);
		return true;
	}

	return false;
}

void UMainUiLayoutWidget::SetWidgetForLayer(const FUiScreenInfo& UiScreenInfo, const bool bCleanUpExistingScreens)
{
	if (bCleanUpExistingScreens)
	{
		for (const FLayerInfo& Layer : Layers)
		{
			TObjectPtr<ULayerWidget> LayerWidget = Layer.LayerWidget;
			if (IsValid(LayerWidget))
			{
				LayerWidget->ClearWidgets();
			}
		}
	}
	else
	{
		RemoveScreensFromHigherLayer(UiScreenInfo);
		if (TrySwitchToExistingScreenInLayer(UiScreenInfo))
		{
			return;
		}
	}

	ULayerWidget* LayerWidget = GetLayerForScreenInfo(UiScreenInfo);

	if (IsValid(LayerWidget))
	{
		LayerWidget->AddWidget(UiScreenInfo.ScreenClass.Get());
	}
}

void UMainUiLayoutWidget::RemoveScreensFromHigherLayer(const FUiScreenInfo& UiScreenInfo)
{
	const FGameplayTag TargetLayerId = UiScreenInfo.LayerId;

	const int32 TargetLayerIndex = Layers.IndexOfByPredicate(
		[&TargetLayerId](const FLayerInfo& Info)
		{
			return Info.LayerId == TargetLayerId;
		});

	if (TargetLayerIndex == INDEX_NONE)
	{
		UE_LOG(LogUiScreenFramework, Error, TEXT("%hs: Layer %s is not registered in the main layout widget."), __FUNCTION__, *TargetLayerId.ToString());
		return;
	}

	for (int32 Index = TargetLayerIndex + 1; Index < Layers.Num(); ++Index)
	{
		if (ULayerWidget* HigherLayerWidget = Layers[Index].LayerWidget)
		{
			if (IsValid(HigherLayerWidget))
			{
				UE_LOG(LogUiScreenFramework, Log, TEXT("%hs: Layer %s content is cleared."), __FUNCTION__, *GetNameSafe(HigherLayerWidget));
				HigherLayerWidget->ClearWidgets();
			}
		}
	}
}

UCommonActivatableWidget* UMainUiLayoutWidget::GetCurrentScreenWidget()
{
	return CurrentScreenWidget.IsValid() ? CurrentScreenWidget.Get() : nullptr;
}

void UMainUiLayoutWidget::NativeDestruct()
{
	for (const FLayerInfo& Layer : Layers)
	{
		TObjectPtr<ULayerWidget> LayerWidget = Layer.LayerWidget;
		if (IsValid(LayerWidget))
		{
			LayerWidget->OnDisplayedWidgetChanged().RemoveAll(this);
			LayerWidget->ClearWidgets();
		}
	}

	Super::NativeConstruct();
}

void UMainUiLayoutWidget::OnDisplayedWidgetChanged(UCommonActivatableWidget* CommonActivatableWidget)
{
	CurrentScreenWidget = CommonActivatableWidget;
	OnDisplayedWidgetChangedDelegate.ExecuteIfBound(CommonActivatableWidget);
}

void UMainUiLayoutWidget::RegisterLayer(FGameplayTag LayerTag, ULayerWidget* LayerWidget)
{
	if (IsDesignTime())
	{
		return;
	}

	if (!IsValid(LayerWidget))
	{
		UE_LOG(LogUiScreenFramework, Warning, TEXT("%hs: Attempted to register an invalid LayerWidget for tag %s."), __FUNCTION__, *LayerTag.ToString());
		return;
	}

	if (Layers.FindByPredicate([&LayerTag](const FLayerInfo& Info) { return Info.LayerId == LayerTag; }))
	{
		UE_LOG(LogUiScreenFramework, Warning, TEXT("%hs: Layer %s is already registered."), __FUNCTION__, *LayerTag.ToString());
		return;
	}

	LayerWidget->OnDisplayedWidgetChanged().AddUObject(this, &UMainUiLayoutWidget::OnDisplayedWidgetChanged);

	Layers.Emplace(FLayerInfo{LayerTag, LayerWidget});
}

void UMainUiLayoutWidget::RegisterTooltipLayer(UOverlay* LayerWidget)
{
	if (!LayerWidget)
	{
		UE_LOG(LogUiScreenFramework, Warning, TEXT("%hs: Attempted to register an invalid LayerWidget for the tooltip layer."), __FUNCTION__);
		return;
	}

	TooltipLayer = LayerWidget;
}
