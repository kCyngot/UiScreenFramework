// Copyright People Can Fly. All Rights Reserved.

#pragma once

#include "Components/IndicatorComponent.h"
#include "GameFramework/Controller.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Delegates/DelegateCombinations.h"

#include "IndicatorManagerSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUIIndicatorPanel, Log, All);

class UBaseIndicatorViewModel;

/**
 * @class UIndicatorManagerSubsystem
 */
UCLASS()
class UISCREENFRAMEWORK_API UIndicatorManagerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	virtual void Deinitialize() override;

	static UIndicatorManagerSubsystem* Get(const APlayerController* PlayerController);

	void AddIndicator(UBaseIndicatorViewModel* IndicatorViewModel);
	
	void RemoveIndicator(UBaseIndicatorViewModel* IndicatorViewModel);

	DECLARE_EVENT_OneParam(UIndicatorManagerSubsystem, FIndicatorEvent, UBaseIndicatorViewModel* IndicatorViewModel)
	FIndicatorEvent OnIndicatorAdded;
	FIndicatorEvent OnIndicatorRemoved;

	DECLARE_EVENT_OneParam(UIndicatorManagerSubsystem, FIndicatorVisibilityEvent, int32)
	FIndicatorVisibilityEvent OnIndicatorCategoryVisibilityChanged;
	
	const TArray<UBaseIndicatorViewModel*>& GetIndicators() const { return Indicators; }

	int32 GetIndicatorVisibilityOption() const { return IndicatorVisibilityOption; }

	// set new visibility option for indicators
	UFUNCTION(BlueprintCallable)
	void SetIndicatorVisibilityOption(UPARAM(meta = (Bitmask, BitmaskEnum = EIndicatorCategory)) const int32 NewVisibilityOption);

#if !UE_BUILD_SHIPPING
	static FString GetIndicatorDebugInfo(const UBaseIndicatorViewModel* IndicatorViewModel, const bool bShowExtended = false);
	void DumpIndicatorPanelInfo(const bool bShowOnScreen = false, const bool bShowExtended = false, const float DisplayTime = 20.0f) const;
	FString GetIndicatorPanelInfo(const bool bShowExtended = true) const;
#endif

private:
	void HandleVisibilityCheck();
	void HandleDistanceCheck();

	UPROPERTY(Transient)
	TArray<UBaseIndicatorViewModel*> Indicators;
	
	// Control the visibility of button parts
	UPROPERTY()
	int32 IndicatorVisibilityOption = static_cast<int32>(EIndicatorCategory::All);
	
	FTimerHandle VisibilityCheckTimerHandle;
	FTimerHandle DistanceCheckTimerHandle;

	FDelegateHandle OnWorldCleanupHandle;
	void OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);
};
