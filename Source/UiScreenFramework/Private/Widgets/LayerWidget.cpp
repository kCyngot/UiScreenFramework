// Copyright People Can Fly. All Rights Reserved."

#include "Widgets/LayerWidget.h"

#include "CommonActivatableWidget.h"
#include "Slate/SCommonAnimatedSwitcher.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SSpacer.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LayerWidget)

DEFINE_LOG_CATEGORY(LogLayerWidget);

UCommonActivatableWidget* GetActivatableWidgetFromSlate(const TSharedPtr<SWidget>& SlateWidget)
{
	if (SlateWidget && SlateWidget != SNullWidget::NullWidget && ensure(SlateWidget->GetType().IsEqual(TEXT("SObjectWidget"))))
	{
		UCommonActivatableWidget* ActivatableWidget = Cast<UCommonActivatableWidget>(StaticCastSharedPtr<SObjectWidget>(SlateWidget)->GetWidgetObject());
		if (ensure(ActivatableWidget))
		{
			return ActivatableWidget;
		}
	}
	return nullptr;
}

ULayerWidget::ULayerWidget(const FObjectInitializer& Initializer)
	: Super(Initializer)
	  , GeneratedWidgetsPool(*this)
{
	SetVisibilityInternal(ESlateVisibility::Collapsed);
}

UCommonActivatableWidget* ULayerWidget::GetActiveWidget() const
{
	return MySwitcher ? GetActivatableWidgetFromSlate(MySwitcher->GetActiveWidget()) : nullptr;
}

int32 ULayerWidget::GetNumWidgets() const
{
	return WidgetList.Num();
}

void ULayerWidget::RemoveWidget(UCommonActivatableWidget& WidgetToRemove)
{
	UE_LOG(LogLayerWidget, Verbose, TEXT("%hs WidgetToRemove: %s"), __FUNCTION__, *WidgetToRemove.GetName());

	if (&WidgetToRemove == GetActiveWidget())
	{
		if (WidgetToRemove.IsActivated())
		{
			WidgetToRemove.DeactivateWidget();
		}
		else
		{
			UE_LOG(LogLayerWidget, Verbose, TEXT("%hs bRemoveDisplayedWidgetPostTransition is true"), __FUNCTION__);
			bRemoveDisplayedWidgetPostTransition = true;
		}
	}
	else
	{
		// Otherwise if the widget isn't actually being shown right now, yank it right on out
		TSharedPtr<SWidget> CachedWidget = WidgetToRemove.GetCachedWidget();
		if (CachedWidget && MySwitcher)
		{
			ReleaseWidget(CachedWidget.ToSharedRef());
		}
		else
		{
			GeneratedWidgetsPool.Release(&WidgetToRemove, true);
			WidgetList.Remove(&WidgetToRemove);
		}
	}
}

void ULayerWidget::ClearWidgets()
{
	SetSwitcherIndex(0);
}

TSharedRef<SWidget> ULayerWidget::RebuildWidget()
{
	MyOverlay = SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SAssignNew(MySwitcher, SCommonAnimatedSwitcher)
			.TransitionCurveType(TransitionCurveType)
			.TransitionDuration(TransitionDuration)
			.TransitionType(TransitionType)
			.TransitionFallbackStrategy(TransitionFallbackStrategy)
			.OnActiveIndexChanged_UObject(this, &ULayerWidget::HandleActiveIndexChanged)
			.OnIsTransitioningChanged_UObject(this, &ULayerWidget::HandleSwitcherIsTransitioningChanged)
		]
		+ SOverlay::Slot()
		[
			SAssignNew(MyInputGuard, SSpacer)
			.Visibility(EVisibility::Collapsed)
		];

	// We always want a 0th slot to be able to animate the first real entry in and out
	MySwitcher->AddSlot()[SNullWidget::NullWidget];

	return MyOverlay.ToSharedRef();
}

void ULayerWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyOverlay.Reset();
	MyInputGuard.Reset();
	MySwitcher.Reset();
	ReleasedWidgets.Empty();
	WidgetList.Reset();

	GeneratedWidgetsPool.ReleaseAll(true);
}

void ULayerWidget::OnWidgetRebuilt()
{
	Super::OnWidgetRebuilt();

	if (!IsDesignTime())
	{
		HandleActiveIndexChanged(0);
	}
}

void ULayerWidget::SetSwitcherIndex(int32 TargetIndex, bool bInstantTransition /*= false*/)
{
	UE_LOG(LogLayerWidget, Verbose, TEXT("%hs TargetIndex: %d, bInstantTransition: %d, IsTransitionPlaying() %s, GetActiveWidgetIndex() %d"), __FUNCTION__, TargetIndex, bInstantTransition,
		MySwitcher->IsTransitionPlaying() ? TEXT("true") : TEXT("false"), MySwitcher->GetActiveWidgetIndex());

	if (MySwitcher)
	{
		if (DisplayedWidget && MySwitcher->GetActiveWidgetIndex() != TargetIndex)
		{
			DisplayedWidget->OnDeactivated().RemoveAll(this);
			if (DisplayedWidget->IsActivated())
			{
				DisplayedWidget->DeactivateWidget();
			}
			else if (MySwitcher->GetActiveWidgetIndex() != 0)
			{
				bRemoveDisplayedWidgetPostTransition = true;
			}
		}

		MySwitcher->TransitionToIndex(TargetIndex, MySwitcher->IsTransitionPlaying() ? true : bInstantTransition);
	}
}

UCommonActivatableWidget* ULayerWidget::AddWidgetInternal(TSubclassOf<UCommonActivatableWidget> ActivatableWidgetClass, TFunctionRef<void(UCommonActivatableWidget&)> InitFunc)
{
	if (UCommonActivatableWidget* WidgetInstance = GeneratedWidgetsPool.GetOrCreateInstance(ActivatableWidgetClass))
	{
		InitFunc(*WidgetInstance);
		RegisterInstanceInternal(*WidgetInstance);
		return WidgetInstance;
	}
	return nullptr;
}

void ULayerWidget::RegisterInstanceInternal(UCommonActivatableWidget& NewWidget)
{
	UE_LOG(LogLayerWidget, Verbose, TEXT("%hs NewWidget: %s"), __FUNCTION__, *NewWidget.GetName());

	if (ensure(!WidgetList.Contains(&NewWidget)))
	{
		WidgetList.Add(&NewWidget);
		OnWidgetAddedToList(NewWidget);
	}
}

void ULayerWidget::HandleSwitcherIsTransitioningChanged(bool bIsTransitioning)
{
	UE_LOG(LogLayerWidget, Verbose, TEXT("%hs bIsTransitioning: %d"), __FUNCTION__, bIsTransitioning);

	// While the switcher is transitioning, put up the guard to intercept all input
	MyInputGuard->SetVisibility(bIsTransitioning ? EVisibility::Visible : EVisibility::Collapsed);
	OnTransitioningChanged.Broadcast(this, bIsTransitioning);
}

void ULayerWidget::HandleActiveWidgetDeactivated(UCommonActivatableWidget* DeactivatedWidget)
{
	UE_LOG(LogLayerWidget, Verbose, TEXT("%hs DeactivatedWidget: %s"), __FUNCTION__, *DeactivatedWidget->GetName());

	// When the currently displayed widget deactivates, transition the switcher to the preceding slot (if it exists)
	// We'll clean up this slot once the switcher index actually changes
	if (ensure(DeactivatedWidget == DisplayedWidget) && MySwitcher && MySwitcher->GetActiveWidgetIndex() > 0)
	{
		DisplayedWidget->OnDeactivated().RemoveAll(this);
		MySwitcher->TransitionToIndex(MySwitcher->GetActiveWidgetIndex() - 1);
	}
}

void ULayerWidget::ReleaseWidget(const TSharedRef<SWidget>& WidgetToRelease)
{
	if (UCommonActivatableWidget* ActivatableWidget = GetActivatableWidgetFromSlate(WidgetToRelease))
	{
		UE_LOG(LogLayerWidget, Verbose, TEXT("%hs WidgetToRelease: %s"), __FUNCTION__, *ActivatableWidget->GetName());

		GeneratedWidgetsPool.Release(ActivatableWidget, true);
		WidgetList.Remove(ActivatableWidget);
	}
	else
	{
		UE_LOG(LogLayerWidget, Warning, TEXT("%hs No matching Activatable Widget found."), __FUNCTION__);
	}

	const int32 RemovedIndex = MySwitcher->RemoveSlot(WidgetToRelease);
	if (RemovedIndex != INDEX_NONE)
	{
		UE_LOG(LogLayerWidget, Verbose, TEXT("%hs Widget removed from slot %d"), __FUNCTION__, RemovedIndex);

		ReleasedWidgets.Add(WidgetToRelease);
		if (ReleasedWidgets.Num() == 1)
		{
			FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this,
				[this](float)
				{
					QUICK_SCOPE_CYCLE_COUNTER(STAT_UCommonActivatableWidgetContainerBase_ReleaseWidget);
					ReleasedWidgets.Reset();
					return false;
				}));
		}
	}
}

void ULayerWidget::HandleActiveIndexChanged(int32 ActiveWidgetIndex)
{
	UE_LOG(LogLayerWidget, Verbose, TEXT("%hs ActiveWidgetIndex: %d"), __FUNCTION__, ActiveWidgetIndex);

	// Remove all slots above the currently active one and release the widgets back to the pool
	while (MySwitcher->GetNumWidgets() - 1 > ActiveWidgetIndex)
	{
		TSharedPtr<SWidget> WidgetToRelease = MySwitcher->GetWidget(MySwitcher->GetNumWidgets() - 1);

		if (ensure(WidgetToRelease))
		{
			ReleaseWidget(WidgetToRelease.ToSharedRef());
		}
	}

	// Also remove the widget that we just transitioned away from if desired
	if (DisplayedWidget && bRemoveDisplayedWidgetPostTransition)
	{
		if (TSharedPtr<SWidget> DisplayedSlateWidget = DisplayedWidget->GetCachedWidget())
		{
			UE_LOG(LogLayerWidget, Verbose, TEXT("Remove the widget that we just transitioned away from: %s, bRemoveDisplayedWidgetPostTransition %s"), *GetNameSafe(DisplayedWidget),
				bRemoveDisplayedWidgetPostTransition ? TEXT("true") : TEXT("false"));
			ReleaseWidget(DisplayedSlateWidget.ToSharedRef());
		}
	}

	bRemoveDisplayedWidgetPostTransition = false;

	// Activate the widget that's now being displayed
	DisplayedWidget = GetActivatableWidgetFromSlate(MySwitcher->GetActiveWidget());
	if (DisplayedWidget)
	{
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		DisplayedWidget->OnDeactivated().AddUObject(this, &ULayerWidget::HandleActiveWidgetDeactivated, ToRawPtr(DisplayedWidget));
		DisplayedWidget->ActivateWidget();

		if (UWorld* MyWorld = GetWorld())
		{
			FTimerManager& TimerManager = MyWorld->GetTimerManager();
			TimerManager.SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]() { InvalidateLayoutAndVolatility(); }));
		}
	}
	else
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}

	OnDisplayedWidgetChanged().Broadcast(DisplayedWidget);
}

void ULayerWidget::SetTransitionDuration(float Duration)
{
	TransitionDuration = Duration;
	if (MySwitcher.IsValid())
	{
		MySwitcher->SetTransition(TransitionDuration, TransitionCurveType);
	}
}

float ULayerWidget::GetTransitionDuration() const
{
	return TransitionDuration;
}

//////////////////////////////////////////////////////////////////////////
// ULayerWidgetStack
//////////////////////////////////////////////////////////////////////////

UCommonActivatableWidget* ULayerWidgetStack::GetRootContent() const
{
	return RootContentWidget;
}

void ULayerWidgetStack::SynchronizeProperties()
{
	Super::SynchronizeProperties();

#if WITH_EDITOR
	if (IsDesignTime() && RootContentWidget && RootContentWidget->GetClass() != RootContentWidgetClass)
	{
		// At design time, account for the possibility of the preview class changing
		if (RootContentWidget->GetCachedWidget())
		{
			MySwitcher->GetChildSlot(0)->DetachWidget();
		}

		RootContentWidget = nullptr;
	}
#endif

	if (!RootContentWidget && RootContentWidgetClass)
	{
		RootContentWidget = CreateWidget<UCommonActivatableWidget>(this, RootContentWidgetClass);
		MySwitcher->GetChildSlot(0)->AttachWidget(RootContentWidget->TakeWidget());
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void ULayerWidgetStack::OnWidgetAddedToList(UCommonActivatableWidget& AddedWidget)
{
	if (MySwitcher)
	{
		MySwitcher->AddSlot()[AddedWidget.TakeWidget()];

		SetSwitcherIndex(MySwitcher->GetNumWidgets() - 1);
	}
}
