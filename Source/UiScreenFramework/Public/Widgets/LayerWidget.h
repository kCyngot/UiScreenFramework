// LayerWidget.h

#pragma once
//
// #include "CoreMinimal.h"
// #include "CommonLazyWidget.h"
// #include "LayerWidget.generated.h"
//
// UCLASS()
// class UISCREENFRAMEWORK_API ULayerWidget : public UCommonLazyWidget
// {
// 	GENERATED_BODY()
// };

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Engine/StreamableManager.h"
#include "Containers/Ticker.h"
#include "LayerWidget.generated.h"

class SOverlay;
class UUserWidget;

UCLASS()
class UISCREENFRAMEWORK_API ULayerWidget : public UWidget
{
	GENERATED_BODY()

public:
	ULayerWidget();

	/**
	 * @brief Asynchronously loads and displays a new widget, with a fade transition.
	 * @param SoftWidget The widget class to load and display.
	 */
	UFUNCTION(BlueprintCallable, Category = "Lazy Content")
	void SetLazyContent(const TSoftClassPtr<UUserWidget> SoftWidget);

	/** @return The currently visible (or fading in) user widget. */
	UFUNCTION(BlueprintCallable, Category = "Lazy Content")
	UUserWidget* GetCurrentContent() const { return CurrentContent; }

	/** @return True if a screen is currently being loaded or a transition is in progress. */
	UFUNCTION(BlueprintCallable, Category = "Lazy Content")
	bool IsInTransition() const;

protected:
	//~ Begin UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~ End UWidget Interface

private:
	/** Starts the process of transitioning to a newly loaded widget instance. */
	void BeginTransition(UUserWidget* NewWidget);

	/** Fades out the old content by registering a ticker. */
	void StartFadeOut();

	/** Updates the fade-out animation. Runs on every tick. Returns false when complete. */
	bool UpdateFadeOut(float DeltaTime);

	/** Called when the fade-out is complete. */
	void OnFadeOutFinished();

	/** Fades in the new content by registering a ticker. */
	void StartFadeIn();

	/** Updates the fade-in animation. Runs on every tick. Returns false when complete. */
	bool UpdateFadeIn(float DeltaTime);

	/** Called when the fade-in is complete. */
	void OnFadeInFinished();

	/** Cancels any ongoing asset streaming and removes active tickers. */
	void CancelTransition();
	
	/** Async loading request wrapper. */
	void RequestAsyncLoad(TSoftClassPtr<UObject> SoftObject, TFunction<void()>&& Callback);

public:
	/** The duration of the fade-in and fade-out animations. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	float TransitionDuration = 0.25f;

private:
	/** The root Slate widget, which allows us to overlay widgets for fading. */
	TSharedPtr<SOverlay> MyOverlay;

	/** The widget currently being displayed or fading in. */
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> CurrentContent;

	/** The widget that is currently fading out. */
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> OldContent;
	
	/** Slate widget representation of the current content. */
	TSharedPtr<SWidget> CurrentContentWidget;

	/** Slate widget representation of the old content. */
	TSharedPtr<SWidget> OldContentWidget;
	
	/** Handle for the ticker delegate, used to start/stop the fade animations. */
	FTSTicker::FDelegateHandle TickerHandle;

	/** Time elapsed during the current fade animation. */
	float CurrentFadeTime = 0.0f;
	
	/** Handle for the asynchronous asset loading. */
	TSharedPtr<FStreamableHandle> StreamingHandle;
	
	/** Path of the asset being loaded, used to prevent race conditions. */
	FSoftObjectPath StreamingObjectPath;
};
