#pragma once

#include "Blueprint/UserWidgetPool.h"
#include "Slate/SCommonAnimatedSwitcher.h"
#include "LayerWidget.generated.h"

class SCommonAnimatedSwitcher;
enum class ECommonSwitcherTransition : uint8;
enum class ETransitionCurve : uint8;

class UCommonActivatableWidget;
class SOverlay;
class SSpacer;

DECLARE_LOG_CATEGORY_EXTERN(LogLayerWidget, Log, All);

/** 
 * Base of widgets built to manage N activatable widgets, displaying one at a time.
 * Intentionally meant to be black boxes that do not expose child/slot modification like a normal panel widget.
 */
UCLASS(Abstract)
class UISCREENFRAMEWORK_API ULayerWidget : public UWidget
{
	GENERATED_BODY()

public:
	ULayerWidget(const FObjectInitializer& Initializer);

	/** Adds an activatable widget to the container. See BP_AddWidget for more info. */
	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	ActivatableWidgetT* AddWidget(TSubclassOf<UCommonActivatableWidget> ActivatableWidgetClass)
	{
		// Don't actually add the widget if the cast will fail
		if (ActivatableWidgetClass && ActivatableWidgetClass->IsChildOf<ActivatableWidgetT>())
		{
			return Cast<ActivatableWidgetT>(AddWidgetInternal(ActivatableWidgetClass, [](UCommonActivatableWidget&) {}));
		}
		return nullptr;
	}

	/** 
	 * Generates (either creates or pulls from the inactive pool) instance of the given widget class and adds it to the container.
	 * The provided lambda is called after the instance has been generated and before it is actually added to the container.
	 * So if you've got setup to do on the instance before it potentially activates, the lambda is the place to do it.
	 */
	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	ActivatableWidgetT* AddWidget(TSubclassOf<UCommonActivatableWidget> ActivatableWidgetClass, TFunctionRef<void(ActivatableWidgetT&)> InstanceInitFunc)
	{
		// Don't actually add the widget if the cast will fail
		if (ActivatableWidgetClass && ActivatableWidgetClass->IsChildOf<ActivatableWidgetT>())
		{
			return Cast<ActivatableWidgetT>(AddWidgetInternal(ActivatableWidgetClass, [&InstanceInitFunc] (UCommonActivatableWidget& WidgetInstance) 
				{
					InstanceInitFunc(*CastChecked<ActivatableWidgetT>(&WidgetInstance));
				}));
		}
		return nullptr;
	}

	/** 
	 * Adds an activatable widget instance to the container. 
	 * This instance is not pooled in any way by the stack and responsibility for ownership lies with the original creator of the widget.
	 * 
	 * NOTE: In general, it is *strongly* recommended that you opt for the class-based AddWidget above. This one is mostly just here for legacy support.
	 */
	void AddWidgetInstance(UCommonActivatableWidget& ActivatableWidget);

	void RemoveWidget(UCommonActivatableWidget& WidgetToRemove);

	UFUNCTION(BlueprintCallable, Category = ActivatableWidgetStack)
	UCommonActivatableWidget* GetActiveWidget() const;

	const TArray<UCommonActivatableWidget*>& GetWidgetList() const { return WidgetList; }

	int32 GetNumWidgets() const;
	void SetSwitcherIndex(int32 TargetIndex, bool bInstantTransition = false);

	UFUNCTION(BlueprintCallable, Category = ActivatableWidgetContainer)
	void ClearWidgets();

	UFUNCTION(BlueprintCallable, Category = ActivatableWidgetContainer)
	void SetTransitionDuration(float Duration);
	UFUNCTION(BlueprintCallable, Category = ActivatableWidgetContainer)
	float GetTransitionDuration() const;

	DECLARE_EVENT_OneParam(UCommonActivatableWidgetContainerBase, FOnDisplayedWidgetChanged, UCommonActivatableWidget*);
	FOnDisplayedWidgetChanged& OnDisplayedWidgetChanged() const { return OnDisplayedWidgetChangedEvent; }

	DECLARE_EVENT_TwoParams(UCommonActivatableWidgetContainerBase, FTransitioningChanged, ULayerWidget* /*Widget*/, bool /*bIsTransitioning*/);
	FTransitioningChanged OnTransitioningChanged;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual void OnWidgetRebuilt() override;

	virtual void OnWidgetAddedToList(UCommonActivatableWidget& AddedWidget) { unimplemented(); }

	/** The type of transition to play between widgets */
	UPROPERTY(EditAnywhere, Category = Transition)
	ECommonSwitcherTransition TransitionType;

	/** The curve function type to apply to the transition animation */
	UPROPERTY(EditAnywhere, Category = Transition)
	ETransitionCurve TransitionCurveType;

	/** The total duration of a single transition between widgets */
	UPROPERTY(EditAnywhere, Category = Transition)
	float TransitionDuration = 0.4f;

	/**
	 * Controls how we will choose another widget if a transitioning widget is removed during the transition.
	 * Note for Queues and Stacks, ECommonSwitcherTransitionFallbackStrategy::Previous is a good option.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Transition)
	ECommonSwitcherTransitionFallbackStrategy TransitionFallbackStrategy = ECommonSwitcherTransitionFallbackStrategy::None;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UCommonActivatableWidget>> WidgetList;

	UPROPERTY(Transient)
	TObjectPtr<UCommonActivatableWidget> DisplayedWidget;

	UPROPERTY(Transient)
	FUserWidgetPool GeneratedWidgetsPool;

	TSharedPtr<SOverlay> MyOverlay;
	TSharedPtr<SSpacer> MyInputGuard;
	TSharedPtr<SCommonAnimatedSwitcher> MySwitcher;

private:
	/** 
	 * Adds a widget of the given class to the container. 
	 * Note that all widgets added to the container are pooled, so the caller should not try to cache and re-use the created widget.
	 * 
	 * It is possible for multiple instances of the same class to be added to the container at once, so any instance created in the past
	 * is not guaranteed to be the one returned this time.
	 *
	 * So in practice, you should not trust that any prior state has been retained on the returned widget, and establish all appropriate properties every time.
	 */
	UFUNCTION(BlueprintCallable, Category = ActivatableWidgetStack, meta = (DeterminesOutputType = ActivatableWidgetClass, DisplayName = "Push Widget"))
	UCommonActivatableWidget* BP_AddWidget(TSubclassOf<UCommonActivatableWidget> ActivatableWidgetClass);

	UFUNCTION(BlueprintCallable, Category = ActivatableWidgetContainer)
	void RemoveWidget(UCommonActivatableWidget* WidgetToRemove);

	UCommonActivatableWidget* AddWidgetInternal(TSubclassOf<UCommonActivatableWidget> ActivatableWidgetClass, TFunctionRef<void(UCommonActivatableWidget&)> InitFunc);
	void RegisterInstanceInternal(UCommonActivatableWidget& NewWidget);

	void HandleSwitcherIsTransitioningChanged(bool bIsTransitioning);
	void HandleActiveIndexChanged(int32 ActiveWidgetIndex);
	void HandleActiveWidgetDeactivated(UCommonActivatableWidget* DeactivatedWidget);

	void ReleaseWidget(const TSharedRef<SWidget>& WidgetToRelease);
	TArray<TSharedPtr<SWidget>> ReleasedWidgets;

	bool bRemoveDisplayedWidgetPostTransition = false;

	mutable FOnDisplayedWidgetChanged OnDisplayedWidgetChangedEvent;
};

//////////////////////////////////////////////////////////////////////////
// ULayerWidgetStack
//////////////////////////////////////////////////////////////////////////

/** 
 * A display stack of ActivatableWidget elements. 
 * 
 * - Only the widget at the top of the stack is displayed and activated. All others are deactivated.
 * - When that top-most displayed widget deactivates, it's automatically removed and the preceding entry is displayed/activated.
 * - If RootContent is provided, it can never be removed regardless of activation state
 */
UCLASS()
class UISCREENFRAMEWORK_API ULayerWidgetStack : public ULayerWidget
{
	GENERATED_BODY()

public:

	UCommonActivatableWidget* GetRootContent() const;

protected:
	virtual void SynchronizeProperties() override;
	virtual void OnWidgetAddedToList(UCommonActivatableWidget& AddedWidget) override;
	
private:
	/** Optional widget to auto-generate as the permanent root element of the stack */
	UPROPERTY(EditAnywhere, Category = Content)
	TSubclassOf<UCommonActivatableWidget> RootContentWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UCommonActivatableWidget> RootContentWidget;
};
