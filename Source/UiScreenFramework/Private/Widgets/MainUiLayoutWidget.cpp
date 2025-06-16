// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/MainUiLayoutWidget.h"

#include "CommonLazyWidget.h"
#include "Logging/LogUiScreenManager.h"
UE_DISABLE_OPTIMIZATION

void UMainUiLayoutWidget::SetWidgetForLayer(const FUiScreenInfo& UiScreenInfo)
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
		if (UCommonLazyWidget* HigherLayerWidget = Layers[Index].LayerWidget)
		{
			if (IsValid(HigherLayerWidget))
			{
				// Setting content to nullptr will clear the widget.
				UE_LOG(LogUiScreenFramework, Log, TEXT("%hs: Layer %s content is cleared."), __FUNCTION__, *GetNameSafe(HigherLayerWidget));
				HigherLayerWidget->SetLazyContent(nullptr);
			}
		}
	}

	if (UCommonLazyWidget* TargetLayerWidget = Layers[TargetLayerIndex].LayerWidget)
	{
		if (IsValid(TargetLayerWidget))
		{
			TargetLayerWidget->SetLazyContent(UiScreenInfo.ScreenClass);
		}
		else
		{
			UE_LOG(LogUiScreenFramework, Error, TEXT("%hs: TargetLayerWidget is invalid for layer %s."), __FUNCTION__, *TargetLayerId.ToString());
		}
	}
}

void UMainUiLayoutWidget::NativeDestruct()
{
	for (const FLayerInfo& Layer : Layers)
	{
		TObjectPtr<UCommonLazyWidget> LayerWidget = Layer.LayerWidget;
		if (IsValid(LayerWidget))
		{
			// LayerWidget->OnLoadingStateChanged().RemoveAll(this);
			// LayerWidget->OnContentChanged().RemoveAll(this);
			LayerWidget->SetLazyContent(nullptr);
		}
	}

	Super::NativeConstruct();
}

void UMainUiLayoutWidget::OnLoadingStateChanged(bool bIsLoading)
{
	UE_LOG(LogUiScreenFramework, Log, TEXT("%hs bIsLoading %s"), __FUNCTION__, bIsLoading ? TEXT("true") : TEXT("false"));
}

void UMainUiLayoutWidget::OnContentChanged(UUserWidget* UserWidget, FGameplayTag Layer)
{
	UE_LOG(LogUiScreenFramework, Log, TEXT("%hs UserWidget %s, Layer %s"), __FUNCTION__, *GetNameSafe(UserWidget), *Layer.ToString());
}

void UMainUiLayoutWidget::RegisterLayer(FGameplayTag LayerTag, UCommonLazyWidget* LayerWidget)
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

	// LayerWidget->OnLoadingStateChanged().AddUObject(this, &UMainUiLayoutWidget::OnLoadingStateChanged);
	// LayerWidget->OnContentChanged().AddUObject(this, &UMainUiLayoutWidget::OnContentChanged, LayerTag);

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

UE_ENABLE_OPTIMIZATION
