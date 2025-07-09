// Copyright People Can Fly. All Rights Reserved.

#include "Widgets/SIndicatorCanvas.h"

#include "Layout/ArrangedChildren.h"
#include "Rendering/DrawElements.h"
#include "Subsystems/IndicatorManagerSubsystem.h"
#include "SceneView.h"
#include "Widgets/Layout/SBox.h"
#include "Engine/AssetManager.h"
#include "Engine/GameViewportClient.h"
#include "Helpers/IndicatorProjectionHelper.h"
#include "View/MVVMView.h"

namespace EArrowDirection
{
	enum Type
	{
		Left,
		Top,
		Right,
		Bottom,
		MAX
	};
}

// Angles for the direction of the arrow to display
const float ArrowRotations[EArrowDirection::MAX] =
{
	270.0f,
	0.0f,
	90.0f,
	180.0f
};

// Offsets for the each direction that the arrow can point
const FVector2D ArrowOffsets[EArrowDirection::MAX] =
{
	FVector2D(-1.0f, 0.0f),
	FVector2D(0.0f, -1.0f),
	FVector2D(1.0f, 0.0f),
	FVector2D(0.0f, 1.0f)
};

void SIndicatorCanvas::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObjects(AllIndicators);
	Collector.AddReferencedObjects(InactiveIndicators);
	IndicatorPool.AddReferencedObjects(Collector);
}

void SIndicatorCanvas::FSlot::UpdateVisibilityStatus(bool bVisible, float DeltaTime)
{
	UE_LOG(LogUIIndicatorPanel, VeryVerbose, TEXT("UpdateVisibilityStatus Widget %s, CurrentVisibility %s, NewVisibility %s"), *GetNameSafe(IndicatorPtr->IndicatorWidget.Get()),
		bIsIndicatorVisible ? TEXT("true") : TEXT("false"), bVisible ? TEXT("true") : TEXT("false"));

	if (bIsIndicatorVisible != bVisible)
	{
		UpdateTransition();
		bIsIndicatorVisible = bVisible;
		bDirty = true;
	}

	RefreshVisibility();

	if (bInTransition)
	{
		RefreshRenderOpacity();
		UpdateTimer(DeltaTime);
	}
}

void SIndicatorCanvas::FSlot::UpdateTransition()
{
	if (TransitionTime > 0)
	{
		// if change of visibility happens when there is still transition we start from current opacity but we change it in opposite direction
		ElapsedTransitionTime = bInTransition ? TransitionTime - ElapsedTransitionTime : 0.0f;

		bInTransition = true;
		UE_LOG(LogUIIndicatorPanel, VeryVerbose, TEXT("UpdateTransition Widget %s, ElapsedTransitionTime %f, bInTransition %s"), *GetNameSafe(IndicatorPtr->IndicatorWidget.Get()),
			ElapsedTransitionTime, bInTransition ? TEXT("true") : TEXT("false"));
	}
}

void SIndicatorCanvas::FSlot::RefreshVisibility() const
{
	const bool bIsVisible = (bIsIndicatorVisible || bInTransition) && bHasValidScreenPosition;
	GetWidget()->SetVisibility(bIsVisible ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed);
	UE_LOG(LogUIIndicatorPanel, VeryVerbose, TEXT("RefreshVisibility Widget %s, Visibility %s"), *GetNameSafe(IndicatorPtr->IndicatorWidget.Get()),
		bIsVisible ? TEXT("true") : TEXT("false"));
}

void SIndicatorCanvas::FSlot::RefreshRenderOpacity() const
{
	const float NewRenderOpacity = bIsIndicatorVisible ? (ElapsedTransitionTime / TransitionTime) : (1 - ElapsedTransitionTime / TransitionTime);
	GetWidget()->SetRenderOpacity(NewRenderOpacity);
	UE_LOG(LogUIIndicatorPanel, VeryVerbose, TEXT("RefreshRenderOpacity Widget %s, RenderOpacity %f"), *GetNameSafe(IndicatorPtr->IndicatorWidget.Get()),
		NewRenderOpacity);
}

void SIndicatorCanvas::FSlot::UpdateTimer(float InDeltaTime)
{
	if (ElapsedTransitionTime > TransitionTime)
	{
		bInTransition = false;
	}
	else
	{
		ElapsedTransitionTime += InDeltaTime;
	}
}

void SIndicatorCanvas::Construct(const FArguments& InArgs, const FLocalPlayerContext& InLocalPlayerContext, const FScreenEdgeMarkersTrackArea& InScreenEdgeMarkersTrackArea)
{
	LocalPlayerContext = InLocalPlayerContext;
	ScreenEdgeMarkersTrackArea = InScreenEdgeMarkersTrackArea;

	IndicatorPool.SetWorld(LocalPlayerContext.GetWorld());

	SetCanTick(false);
	SetVisibility(EVisibility::SelfHitTestInvisible);

	UpdateActiveTimer();
}

EActiveTimerReturnType SIndicatorCanvas::UpdateCanvas(double InCurrentTime, float InDeltaTime)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_SIndicatorCanvas_UpdateCanvas);
	if (!OptionalPaintGeometry.IsSet())
	{
		return EActiveTimerReturnType::Continue;
	}

	// Grab the local player
	ULocalPlayer* LocalPlayer = LocalPlayerContext.GetLocalPlayer();
	APlayerController* PlayerController = LocalPlayerContext.GetPlayerController();
	UIndicatorManagerSubsystem* IndicatorManagerSubsystem = IndicatorManager.Get();
	if (IndicatorManagerSubsystem == nullptr)
	{
		IndicatorManagerSubsystem = UIndicatorManagerSubsystem::Get(PlayerController);
		if (IndicatorManagerSubsystem)
		{
			IndicatorManager = IndicatorManagerSubsystem;
			IndicatorManagerSubsystem->OnIndicatorAdded.AddSP(this, &SIndicatorCanvas::OnIndicatorAdded);
			IndicatorManagerSubsystem->OnIndicatorRemoved.AddSP(this, &SIndicatorCanvas::OnIndicatorRemoved);
			OnIndicatorVisibilityChanged(IndicatorManagerSubsystem->GetIndicatorVisibilityOption());
			IndicatorManagerSubsystem->OnIndicatorCategoryVisibilityChanged.AddSP(this, &SIndicatorCanvas::OnIndicatorVisibilityChanged);
			for (UBaseIndicatorViewModel* Indicator : IndicatorManagerSubsystem->GetIndicators())
			{
				OnIndicatorAdded(Indicator);
			}
		}
		else
		{
			return EActiveTimerReturnType::Continue;
		}
	}

	//Make sure we have a player. If we don't, we can't project anything
	if (IsValid(LocalPlayer) && IsValid(PlayerController))
	{
		const FGeometry& PaintGeometry = OptionalPaintGeometry.GetValue();
		const FDeprecateSlateVector2D& GeometrySize = PaintGeometry.Size;

		FSceneViewProjectionData ProjectionData;
		if (LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport, /*out*/ ProjectionData))
		{
			SetShowAnyIndicators(true);

			bool IndicatorsChanged = false;

			for (int32 ChildIndex = 0; ChildIndex < CanvasChildren.Num(); ++ChildIndex)
			{
				SIndicatorCanvas::FSlot& CurChild = CanvasChildren[ChildIndex];
				UBaseIndicatorViewModel* IndicatorViewModel = CurChild.IndicatorPtr.Get();

				if (IndicatorViewModel)
				{
					CurChild.UpdateVisibilityStatus(IndicatorViewModel->GetIndicatorVisibility(), InDeltaTime);

					if (!CurChild.GetIsIndicatorVisible())
					{
						IndicatorsChanged |= CurChild.bIsDirty();
						CurChild.ClearDirtyFlag();
						continue;
					}

					// If the indicator changed clamp status between updates, alert the indicator and mark the indicators as changed
					if (CurChild.WasIndicatorClampedStatusChanged())
					{
						//Indicator->OnIndicatorClampedStatusChanged(CurChild.WasIndicatorClamped());
						CurChild.ClearIndicatorClampedStatusChangedFlag();
						IndicatorsChanged = true;
					}

					FVector2D OutScreenPosition;
					bool bIsOnTheTrack = false;
					float TrackArrowAngle = 0.f;
					FVector WorldPosition;

					const bool Success = IndicatorProjectionHelper::Project(*IndicatorViewModel, ProjectionData, GeometrySize, ScreenEdgeMarkersTrackArea,
						OutScreenPosition, bIsOnTheTrack, TrackArrowAngle, WorldPosition);

					if (!Success)
					{
						CurChild.SetHasValidScreenPosition(false);
						CurChild.SetInFrontOfCamera(false);

						IndicatorsChanged |= CurChild.bIsDirty();
						CurChild.ClearDirtyFlag();
						continue;
					}

					IndicatorViewModel->SetIsIndicatorClamped(bIsOnTheTrack);
					IndicatorViewModel->SetClampAngle(TrackArrowAngle);

					CurChild.SetInFrontOfCamera(Success);
					CurChild.SetHasValidScreenPosition(CurChild.GetInFrontOfCamera() || IndicatorViewModel->GetClampToScreen());

					if (CurChild.HasValidScreenPosition())
					{
						// Only dirty the screen position if we can actually show this indicator.
						CurChild.SetScreenPosition(OutScreenPosition);

						const double Depth = FVector::DistSquared2D(ProjectionData.ViewOrigin, WorldPosition);
						CurChild.SetDepth(Depth);
					}

					CurChild.SetPriority(IndicatorViewModel->GetPriority());

					IndicatorsChanged |= CurChild.bIsDirty();
					CurChild.ClearDirtyFlag();
				}
			}

			if (IndicatorsChanged)
			{
				Invalidate(EInvalidateWidget::Paint);
			}
		}
		else
		{
			SetShowAnyIndicators(false);
		}
	}
	else
	{
		SetShowAnyIndicators(false);
	}

	if (AllIndicators.Num() == 0)
	{
		TickHandle.Reset();
		return EActiveTimerReturnType::Stop;
	}
	else
	{
		return EActiveTimerReturnType::Continue;
	}
}

void SIndicatorCanvas::SetShowAnyIndicators(bool bIndicators)
{
	if (bShowAnyIndicators != bIndicators)
	{
		bShowAnyIndicators = bIndicators;

		if (!bShowAnyIndicators)
		{
			for (int32 ChildIndex = 0; ChildIndex < AllChildren.Num(); ChildIndex++)
			{
				AllChildren.GetChildAt(ChildIndex)->SetVisibility(EVisibility::Collapsed);
			}
		}
	}
}

void SIndicatorCanvas::OnIndicatorVisibilityChanged(int32 NewIndicatorVisibilityOption)
{
	CurrentIndicatorVisibilityOption = NewIndicatorVisibilityOption;
}

void SIndicatorCanvas::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_SIndicatorCanvas_OnArrangeChildren);

	//Make sure we have a player. If we don't, we can't project anything
	if (bShowAnyIndicators)
	{
		const FIntPoint FixedPadding = FIntPoint(10.0f, 10.0f);
		const FVector Center = FVector(AllottedGeometry.Size * 0.5f, 0.0f);

		// Sort the children
		TArray<const SIndicatorCanvas::FSlot*> SortedSlots;
		for (int32 ChildIndex = 0; ChildIndex < CanvasChildren.Num(); ++ChildIndex)
		{
			SortedSlots.Add(&CanvasChildren[ChildIndex]);
		}

		SortedSlots.StableSort(
			[](const SIndicatorCanvas::FSlot& A, const SIndicatorCanvas::FSlot& B)
			{
				return A.GetPriority() == B.GetPriority() ? A.GetDepth() > B.GetDepth() : A.GetPriority() < B.GetPriority();
			}
			);

		// Go through all the sorted children
		for (int32 ChildIndex = 0; ChildIndex < SortedSlots.Num(); ++ChildIndex)
		{
			//grab a child
			const SIndicatorCanvas::FSlot& CurChild = *SortedSlots[ChildIndex];
			const UBaseIndicatorViewModel* IndicatorViewModel = CurChild.IndicatorPtr.Get();

			if (IndicatorViewModel && ShouldIndicatorBeDisplayed(IndicatorViewModel->GetIndicatorCategory()))
			{
				// Skip this indicator if it's invalid or has an invalid world position
				if (!ArrangedChildren.Accepts(CurChild.GetWidget()->GetVisibility()))
				{
					CurChild.SetWasIndicatorClamped(false);
					continue;
				}

				const FVector2D& ScreenPosition = CurChild.GetScreenPosition();
				const bool bInFrontOfCamera = CurChild.GetInFrontOfCamera();

				// Don't bother if we can't project the position and the indicator doesn't want to be clamped
				const bool bShouldClamp = IndicatorViewModel->GetClampToScreen();

				//get the offset and final size of the slot
				FVector2D SlotSize, SlotOffset, SlotPaddingMin, SlotPaddingMax;
				GetOffsetAndSize(IndicatorViewModel, SlotSize, SlotOffset, SlotPaddingMin, SlotPaddingMax);

				// Add the information about this child to the output list (ArrangedChildren)
				ArrangedChildren.AddWidget(
					AllottedGeometry.MakeChild(
						CurChild.GetWidget(),
						ScreenPosition + SlotOffset,
						SlotSize,
						1.f
						)
					);
			}
		}
	}
}

int32 SIndicatorCanvas::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
	const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_SIndicatorCanvas_OnPaint);

	OptionalPaintGeometry = AllottedGeometry;

	FArrangedChildren ArrangedChildren(EVisibility::Visible);
	ArrangeChildren(AllottedGeometry, ArrangedChildren);

	int32 MaxLayerId = LayerId;

	const FPaintArgs NewArgs = Args.WithNewParent(this);
	const bool bShouldBeEnabled = ShouldBeEnabled(bParentEnabled);

	for (const FArrangedWidget& CurWidget : ArrangedChildren.GetInternalArray())
	{
		if (!IsChildWidgetCulled(MyCullingRect, CurWidget))
		{
			SWidget* MutableWidget = const_cast<SWidget*>(&CurWidget.Widget.Get());

			const int32 CurWidgetsMaxLayerId = CurWidget.Widget->Paint(NewArgs, CurWidget.Geometry, MyCullingRect, OutDrawElements, bDrawElementsInOrder ? MaxLayerId : LayerId, InWidgetStyle,
				bShouldBeEnabled);
			MaxLayerId = FMath::Max(MaxLayerId, CurWidgetsMaxLayerId);
		}
	}

	return MaxLayerId;
}

void SIndicatorCanvas::OnIndicatorAdded(UBaseIndicatorViewModel* IndicatorViewModel)
{
	checkf(IndicatorViewModel != nullptr,
		TEXT("This should never happen with gc.PendingKillEnabled=False. If it's still True, test with -DisablePendingKill to see who's leaking the UIndicatorViewModel objects."));

	AllIndicators.Add(IndicatorViewModel);
	InactiveIndicators.Add(IndicatorViewModel);

	AddIndicatorForEntry(IndicatorViewModel);
}

void SIndicatorCanvas::OnIndicatorRemoved(UBaseIndicatorViewModel* IndicatorViewModel)
{
	RemoveIndicatorForEntry(IndicatorViewModel);

	AllIndicators.Remove(IndicatorViewModel);
	InactiveIndicators.Remove(IndicatorViewModel);
}

void SIndicatorCanvas::AddIndicatorForEntry(UBaseIndicatorViewModel* Indicator)
{
	// Async load the indicator, and pool the results so that it's easy to use and reuse the widgets.
	TSoftClassPtr<UUserWidget> IndicatorClass = Indicator->GetIndicatorClass();
	if (!IndicatorClass.IsNull())
	{
		TWeakObjectPtr<UBaseIndicatorViewModel> IndicatorPtr(Indicator);

		UAssetManager::GetStreamableManager().RequestAsyncLoad(IndicatorClass.ToSoftObjectPath(),
			FStreamableDelegate::CreateSP(this, &SIndicatorCanvas::AddIndicatorToSlot, IndicatorClass, IndicatorPtr), FStreamableManager::AsyncLoadHighPriority, false, false,
			TEXT("SIndicatorCanvas::AddIndicatorForEntry"));
	}
}

void SIndicatorCanvas::AddIndicatorToSlot(TSoftClassPtr<UUserWidget> IndicatorWidgetClass, TWeakObjectPtr<UBaseIndicatorViewModel> IndicatorViewModelSoft)
{
	if (UBaseIndicatorViewModel* IndicatorViewModel = IndicatorViewModelSoft.Get())
	{
		// While async loading this indicator widget we could have removed it.
		if (!AllIndicators.Contains(IndicatorViewModel))
		{
			return;
		}

		UE_LOG(LogUIIndicatorPanel, Verbose, TEXT("AddIndicatorToSlot IndicatorClass %s"), *GetNameSafe(IndicatorWidgetClass.Get()));

		// Create the widget from the pool.
		if (UUserWidget* IndicatorWidget = IndicatorPool.GetOrCreateInstance(TSubclassOf<UUserWidget>(IndicatorWidgetClass.Get())))
		{
			IndicatorViewModel->IndicatorWidget = IndicatorWidget;

			UMVVMView* View = IndicatorWidget->GetExtension<UMVVMView>();
			if (IsValid(View))
			{
				// UE_LOG(LogUIIndicatorPanel, Warning, TEXT("%hs Widget %s does not have a view model %s"), __FUNCTION__, *GetNameSafe(IndicatorWidget), *GetNameSafe(Indicator));
				View->SetViewModelByClass(IndicatorViewModel);
			}

			InactiveIndicators.Remove(IndicatorViewModel);

			AddActorSlot(IndicatorViewModel)
			[
				SAssignNew(IndicatorViewModel->CanvasHost, SBox)
				[
					IndicatorWidget->TakeWidget()
				]
			];
		}
	}
}

void SIndicatorCanvas::RemoveIndicatorForEntry(UBaseIndicatorViewModel* Indicator)
{
	if (UUserWidget* IndicatorWidget = Indicator->IndicatorWidget.Get())
	{
		Indicator->IndicatorWidget = nullptr;

		IndicatorPool.Release(IndicatorWidget);
	}

	TSharedPtr<SWidget> CanvasHost = Indicator->CanvasHost.Pin();
	if (CanvasHost.IsValid())
	{
		RemoveActorSlot(CanvasHost.ToSharedRef());
		Indicator->CanvasHost.Reset();
	}
}

SIndicatorCanvas::FScopedWidgetSlotArguments SIndicatorCanvas::AddActorSlot(UBaseIndicatorViewModel* IndicatorViewModel)
{
	TWeakPtr<SIndicatorCanvas> WeakCanvas = SharedThis(this);
	return FScopedWidgetSlotArguments{
		MakeUnique<FSlot>(IndicatorViewModel, IndicatorViewModel->GetTransitionTime()), this->CanvasChildren, INDEX_NONE, [WeakCanvas](const FSlot*, int32)
		{
			if (TSharedPtr<SIndicatorCanvas> Canvas = WeakCanvas.Pin())
			{
				Canvas->UpdateActiveTimer();
			}
		}
	};
}

int32 SIndicatorCanvas::RemoveActorSlot(const TSharedRef<SWidget>& SlotWidget)
{
	for (int32 SlotIdx = 0; SlotIdx < CanvasChildren.Num(); ++SlotIdx)
	{
		if (SlotWidget == CanvasChildren[SlotIdx].GetWidget())
		{
			CanvasChildren.RemoveAt(SlotIdx);

			UpdateActiveTimer();

			return SlotIdx;
		}
	}

	return -1;
}

void SIndicatorCanvas::GetOffsetAndSize(const UBaseIndicatorViewModel* IndicatorViewModel,
	FVector2D& OutSize,
	FVector2D& OutOffset,
	FVector2D& OutPaddingMin,
	FVector2D& OutPaddingMax) const
{
	//This might get used one day
	FVector2D AllottedSize = FVector2D::ZeroVector;

	//grab the desired size of the child widget
	TSharedPtr<SWidget> CanvasHost = IndicatorViewModel->CanvasHost.Pin();
	if (CanvasHost.IsValid())
	{
		OutSize = CanvasHost->GetDesiredSize();
	}

	//handle horizontal alignment
	switch (IndicatorViewModel->GetHAlign())
	{
	case HAlign_Left: // same as Align_Top
		OutOffset.X = 0.0f;
		OutPaddingMin.X = 0.0f;
		OutPaddingMax.X = OutSize.X;
		break;

	case HAlign_Center:
		OutOffset.X = (AllottedSize.X - OutSize.X) / 2.0f;
		OutPaddingMin.X = OutSize.X / 2.0f;
		OutPaddingMax.X = OutPaddingMin.X;
		break;

	case HAlign_Right: // same as Align_Bottom
		OutOffset.X = AllottedSize.X - OutSize.X;
		OutPaddingMin.X = OutSize.X;
		OutPaddingMax.X = 0.0f;
		break;
	}

	//Now, handle vertical alignment
	switch (IndicatorViewModel->GetVAlign())
	{
	case VAlign_Top:
		OutOffset.Y = 0.0f;
		OutPaddingMin.Y = 0.0f;
		OutPaddingMax.Y = OutSize.Y;
		break;

	case VAlign_Center:
		OutOffset.Y = (AllottedSize.Y - OutSize.Y) / 2.0f;
		OutPaddingMin.Y = OutSize.Y / 2.0f;
		OutPaddingMax.Y = OutPaddingMin.Y;
		break;

	case VAlign_Bottom:
		OutOffset.Y = AllottedSize.Y - OutSize.Y;
		OutPaddingMin.Y = OutSize.Y;
		OutPaddingMax.Y = 0.0f;
		break;
	}
}

void SIndicatorCanvas::UpdateActiveTimer()
{
	const bool NeedsTicks = AllIndicators.Num() > 0 || !IndicatorManager.IsValid();

	if (NeedsTicks && !TickHandle.IsValid())
	{
		TickHandle = RegisterActiveTimer(0, FWidgetActiveTimerDelegate::CreateSP(this, &SIndicatorCanvas::UpdateCanvas));
	}
}
