// Copyright People Can Fly. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ViewModels/BaseIndicatorViewModel.h"
#include "Structs/ScreenEdgeMarkersTrackArea.h"
#include "Widgets/SWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SlotBase.h"
#include "Layout/Children.h"
#include "Widgets/SPanel.h"
#include "UObject/WeakInterfacePtr.h"
#include "Blueprint/UserWidgetPool.h"

class FArrangedChildren;
class SIndicatorCanvas;
class UIndicatorManagerSubsystem;;

class SIndicatorCanvas : public SPanel
{
public:
	void AddReferencedObjects(FReferenceCollector& Collector);

	/** ActorCanvas-specific slot class */
	class FSlot : public TSlotBase<FSlot>
	{
	public:
		FSlot(UBaseIndicatorViewModel* InIndicator, float InTransitionTime)
			: TSlotBase<FSlot>()
			  , IndicatorPtr(InIndicator)
			  , ScreenPosition(FVector2D::ZeroVector)
			  , Depth(0)
			  , Priority(0.f)
			  , ElapsedTransitionTime(0)
			  , TransitionTime(InTransitionTime)
			  , bInTransition(false)
			  , bIsIndicatorVisible(false)
			  , bInFrontOfCamera(true)
			  , bHasValidScreenPosition(false)
			  , bDirty(true)
			  , bWasIndicatorClamped(false)
			  , bWasIndicatorClampedStatusChanged(false)
		{
		}

		SLATE_SLOT_BEGIN_ARGS(FSlot, TSlotBase<FSlot>)
		SLATE_SLOT_END_ARGS()

		using TSlotBase<FSlot>::Construct;

		bool GetIsIndicatorVisible() const { return bIsIndicatorVisible || bInTransition; }

		void UpdateVisibilityStatus(bool bVisible, float DeltaTime);

		void UpdateTransition();

		const FVector2D& GetScreenPosition() const { return ScreenPosition; }

		void SetScreenPosition(const FVector2D& InScreenPosition)
		{
			if (ScreenPosition != InScreenPosition)
			{
				ScreenPosition = InScreenPosition;
				bDirty = true;
			}
		}

		double GetDepth() const { return Depth; }

		void SetDepth(double InDepth)
		{
			if (Depth != InDepth)
			{
				Depth = InDepth;
				bDirty = true;
			}
		}

		int32 GetPriority() const { return Priority; }

		void SetPriority(int32 InPriority)
		{
			if (Priority != InPriority)
			{
				Priority = InPriority;
				bDirty = true;
			}
		}

		bool GetInFrontOfCamera() const { return bInFrontOfCamera; }

		void SetInFrontOfCamera(bool bInFront)
		{
			if (bInFrontOfCamera != bInFront)
			{
				bInFrontOfCamera = bInFront;
				bDirty = true;
			}

			RefreshVisibility();
		}

		bool HasValidScreenPosition() const { return bHasValidScreenPosition; }

		void SetHasValidScreenPosition(bool bValidScreenPosition)
		{
			if (bHasValidScreenPosition != bValidScreenPosition)
			{
				bHasValidScreenPosition = bValidScreenPosition;
				bDirty = true;
			}

			RefreshVisibility();
		}

		bool bIsDirty() const { return bDirty; }

		void ClearDirtyFlag()
		{
			bDirty = false;
		}

		bool WasIndicatorClamped() const { return bWasIndicatorClamped; }

		void SetWasIndicatorClamped(bool bWasClamped) const
		{
			if (bWasClamped != bWasIndicatorClamped)
			{
				bWasIndicatorClamped = bWasClamped;
				bWasIndicatorClampedStatusChanged = true;

				// TODO add broadcasting info whether indicator is off screen
			}
		}

		bool WasIndicatorClampedStatusChanged() const { return bWasIndicatorClampedStatusChanged; }

		void ClearIndicatorClampedStatusChangedFlag()
		{
			bWasIndicatorClampedStatusChanged = false;
		}

	private:
		void RefreshVisibility() const;
		void RefreshRenderOpacity() const;
		void UpdateTimer(float InDeltaTime);

		TWeakObjectPtr<UBaseIndicatorViewModel> IndicatorPtr;
		FVector2D ScreenPosition = FVector2D::ZeroVector;
		double Depth = 0;
		int32 Priority = 0;
		float ElapsedTransitionTime = 0;
		float TransitionTime = 0;
		bool bInTransition = false;
		uint8 bIsIndicatorVisible : 1;
		uint8 bInFrontOfCamera : 1;
		uint8 bHasValidScreenPosition : 1;
		uint8 bDirty : 1;

		/**
		 * Cached & frame-deferred value of whether the indicator was visually screen clamped last frame or not;
		 * Semi-hacky mutable implementation as it is cached during a const paint operation
		 */
		mutable uint8 bWasIndicatorClamped : 1;
		mutable uint8 bWasIndicatorClampedStatusChanged : 1;

		friend class SIndicatorCanvas;
	};

	/** Begin the arguments for this slate widget */
	SLATE_BEGIN_ARGS(SIndicatorCanvas)
		{
			_Visibility = EVisibility::HitTestInvisible;
		}

		/** Indicates that we have a slot that this widget supports */
		SLATE_SLOT_ARGUMENT(SIndicatorCanvas::FSlot, Slots)

		/** This always goes at the end */
	SLATE_END_ARGS()

	SIndicatorCanvas()
		: CanvasChildren(this)
		  , AllChildren(this)
	{
		AllChildren.AddChildren(CanvasChildren);
	}

	void Construct(const FArguments& InArgs, const FLocalPlayerContext& InCtx, const FScreenEdgeMarkersTrackArea& InScreenEdgeMarkersTrackArea);

	// SWidget Interface
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D::ZeroVector; }
	virtual FChildren* GetChildren() override { return &AllChildren; }
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
		const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const;
	// End SWidget

	void SetDrawElementsInOrder(bool bInDrawElementsInOrder) { bDrawElementsInOrder = bInDrawElementsInOrder; }

private:
	void OnIndicatorAdded(UBaseIndicatorViewModel* Indicator);
	void OnIndicatorRemoved(UBaseIndicatorViewModel* IndicatorViewModel);

	void AddIndicatorForEntry(UBaseIndicatorViewModel* Indicator);
	void RemoveIndicatorForEntry(UBaseIndicatorViewModel* Indicator);

	using FScopedWidgetSlotArguments = TPanelChildren<FSlot>::FScopedWidgetSlotArguments;
	FScopedWidgetSlotArguments AddActorSlot(UBaseIndicatorViewModel* IndicatorViewModel);
	int32 RemoveActorSlot(const TSharedRef<SWidget>& SlotWidget);

	void SetShowAnyIndicators(bool bIndicators);
	void OnIndicatorVisibilityChanged(int32 NewIndicatorVisibilityOption);
	EActiveTimerReturnType UpdateCanvas(double InCurrentTime, float InDeltaTime);

	void GetOffsetAndSize(const UBaseIndicatorViewModel* IndicatorViewModel,
		FVector2D& OutSize,
		FVector2D& OutOffset,
		FVector2D& OutPaddingMin,
		FVector2D& OutPaddingMax) const;

	void UpdateActiveTimer();

private:
	TArray<TObjectPtr<UBaseIndicatorViewModel>> AllIndicators;
	TArray<TObjectPtr<UBaseIndicatorViewModel>> InactiveIndicators;

	FLocalPlayerContext LocalPlayerContext;
	TWeakObjectPtr<UIndicatorManagerSubsystem> IndicatorManager;

	FScreenEdgeMarkersTrackArea ScreenEdgeMarkersTrackArea;

	/** All the slots in this canvas */
	TPanelChildren<FSlot> CanvasChildren;
	FCombinedChildren AllChildren;

	FUserWidgetPool IndicatorPool;

	/** Whether to draw elements in the order they were added to canvas. Note: Enabling this will disable batching and will cause a greater number of drawcalls */
	bool bDrawElementsInOrder = false;

	bool bShowAnyIndicators = false;
	int32 CurrentIndicatorVisibilityOption = 0;

	bool ShouldIndicatorBeDisplayed(EIndicatorCategory IndicatorCategory) const
	{
		return static_cast<int32>(IndicatorCategory) & CurrentIndicatorVisibilityOption;
	}

	mutable TOptional<FGeometry> OptionalPaintGeometry;

	TSharedPtr<FActiveTimerHandle> TickHandle;

public:
	void AddIndicatorToSlot(TSoftClassPtr<UUserWidget> IndicatorWidgetClass, TWeakObjectPtr<UBaseIndicatorViewModel> IndicatorViewModelSoft);
};
