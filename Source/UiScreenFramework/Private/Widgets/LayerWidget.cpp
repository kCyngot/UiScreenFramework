#include "Widgets/LayerWidget.h"

#include "CommonActivatableWidget.h"
#include "Widgets/SOverlay.h"
#include "Blueprint/UserWidget.h"
#include "Engine/AssetManager.h"
#include "Containers/Ticker.h"
#include "Logging/LogUiScreenManager.h"

ULayerWidget::ULayerWidget()
{
	// Set a default to prevent it being 0
	TransitionDuration = 0.25f;
	if (TransitionDuration <= 0.0f)
	{
		TransitionDuration = 0.01f;
	}
}

TSharedRef<SWidget> ULayerWidget::RebuildWidget()
{
	MyOverlay = SNew(SOverlay);

	// If there's already content when the widget is rebuilt, add it to the overlay
	if (CurrentContent && CurrentContent->GetCachedWidget().IsValid())
	{
		CurrentContentWidget = CurrentContent->GetCachedWidget();
		MyOverlay->AddSlot()
		[
			CurrentContentWidget.ToSharedRef()
		];
	}

	return MyOverlay.ToSharedRef();
}

void ULayerWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	CancelTransition();
	MyOverlay.Reset();
	CurrentContentWidget.Reset();
	OldContentWidget.Reset();
}

bool ULayerWidget::IsInTransition() const
{
	return TickerHandle.IsValid() || StreamingHandle.IsValid();
}

void ULayerWidget::SetLazyContent(const TSoftClassPtr<UUserWidget> SoftWidget)
{
	CancelTransition();

	// If the widget path is null, we just fade out the current screen.
	if (SoftWidget.IsNull())
	{
		BeginTransition(nullptr);
		return;
	}

	// Don't do anything if we're already transitioning to this widget
	if (StreamingObjectPath == SoftWidget.ToSoftObjectPath() || IsInTransition())
	{
		UE_LOG(LogUiScreenFramework, Log, TEXT("%hs StreamingObjectPath %s, SoftWidget.ToSoftObjectPath() %s, IsInTransition() %s"), __FUNCTION__,
			*StreamingObjectPath.ToString(), *SoftWidget.ToSoftObjectPath().ToString(), IsInTransition() ? TEXT("true") : TEXT("false"));
		return;
	}

	// If the requested widget is the one we already have, do nothing.
	if (CurrentContent && CurrentContent->GetClass() == SoftWidget.Get())
	{
		UE_LOG(LogUiScreenFramework, Log, TEXT("%hs CurrentContent->GetClass() %s, SoftWidget.Get() %s"), __FUNCTION__,
			*CurrentContent->GetClass()->GetName(), *SoftWidget.Get()->GetName());
		return;
	}

	TWeakObjectPtr<ULayerWidget> WeakThis(this);
	RequestAsyncLoad(SoftWidget, [WeakThis, SoftWidget]()
	{
		if (ULayerWidget* StrongThis = WeakThis.Get())
		{
			if (ensure(SoftWidget.Get()))
			{
				UUserWidget* NewUserWidget = CreateWidget<UUserWidget>(StrongThis->GetOwningPlayer(), SoftWidget.Get());
				StrongThis->BeginTransition(NewUserWidget);
			}
		}
	});
}

void ULayerWidget::BeginTransition(UUserWidget* NewWidget)
{
	OldContent = CurrentContent;
	CurrentContent = NewWidget;

	if (MyOverlay.IsValid())
	{
		OldContentWidget = (OldContent) ? OldContent->GetCachedWidget() : nullptr;

		if (OldContentWidget.IsValid())
		{
			StartFadeOut();
		}
		else
		{
			StartFadeIn();
		}
	}
}

void ULayerWidget::StartFadeOut()
{
	if (!OldContentWidget.IsValid())
	{
		OnFadeOutFinished();
		return;
	}

	CurrentFadeTime = 0.0f;

	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &ULayerWidget::UpdateFadeOut));
}

bool ULayerWidget::UpdateFadeOut(float DeltaTime)
{
	CurrentFadeTime += DeltaTime;
	const float Alpha = FMath::Clamp(1.0f - (CurrentFadeTime / TransitionDuration), 0.0f, 1.0f);

	if (OldContentWidget.IsValid())
	{
		OldContentWidget->SetRenderOpacity(Alpha);
	}

	if (Alpha <= 0.0f)
	{
		OnFadeOutFinished();
		return false;
	}

	return true;
}

void ULayerWidget::OnFadeOutFinished()
{
	TickerHandle.Reset();

	if (MyOverlay.IsValid() && OldContentWidget.IsValid())
	{
		MyOverlay->RemoveSlot(OldContentWidget.ToSharedRef());
	}

	if (UCommonActivatableWidget* IncomingActivatable = Cast<UCommonActivatableWidget>(OldContent))
	{
		IncomingActivatable->DeactivateWidget();
	}

	OldContentWidget.Reset();
	OldContent = nullptr;

	StartFadeIn();
}

void ULayerWidget::StartFadeIn()
{
	if (!CurrentContent)
	{
		OnFadeInFinished();
		return;
	}

	CurrentContentWidget = CurrentContent->TakeWidget();
	CurrentContentWidget->SetRenderOpacity(0.0f);
	MyOverlay->AddSlot()
	[
		CurrentContentWidget.ToSharedRef()
	];

	CurrentFadeTime = 0.0f;

	if (UCommonActivatableWidget* IncomingActivatable = Cast<UCommonActivatableWidget>(CurrentContent))
	{
		IncomingActivatable->ActivateWidget();
	}

	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &ULayerWidget::UpdateFadeIn));
}

bool ULayerWidget::UpdateFadeIn(float DeltaTime)
{
	CurrentFadeTime += DeltaTime;
	const float Alpha = FMath::Clamp(CurrentFadeTime / TransitionDuration, 0.0f, 1.0f);

	if (CurrentContentWidget.IsValid())
	{
		CurrentContentWidget->SetRenderOpacity(Alpha);
	}

	if (Alpha >= 1.0f)
	{
		OnFadeInFinished();
		return false;
	}

	return true;
}

void ULayerWidget::OnFadeInFinished()
{
	TickerHandle.Reset();

	if (CurrentContentWidget.IsValid())
	{
		CurrentContentWidget->SetRenderOpacity(1.0f);
	}
}

void ULayerWidget::CancelTransition()
{
	if (StreamingHandle.IsValid())
	{
		StreamingHandle->CancelHandle();
		StreamingHandle.Reset();
	}
	StreamingObjectPath.Reset();

	if (TickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
		TickerHandle.Reset();
	}
}

void ULayerWidget::RequestAsyncLoad(TSoftClassPtr<UObject> SoftObject, TFunction<void()>&& Callback)
{
	if (UObject* StrongObject = SoftObject.Get())
	{
		Callback();
		return;
	}

	TWeakObjectPtr<ULayerWidget> WeakThis(this);
	StreamingObjectPath = SoftObject.ToSoftObjectPath();
	StreamingHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		StreamingObjectPath,
		[WeakThis, Callback = MoveTemp(Callback), SoftObject]()
		{
			if (ULayerWidget* StrongThis = WeakThis.Get())
			{
				if (StrongThis->StreamingObjectPath != SoftObject.ToSoftObjectPath())
				{
					return;
				}

				Callback();
				StrongThis->StreamingHandle.Reset();
			}
		});
}
