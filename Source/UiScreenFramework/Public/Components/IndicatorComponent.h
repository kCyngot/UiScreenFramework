// Copyright People Can Fly. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ViewModels/BaseIndicatorViewModel.h"
#include "Components/ActorComponent.h"
#include "IndicatorComponent.generated.h"

class UUserWidget;
class UIndicatorManagerSubsystem;

UCLASS(ClassGroup=(Indicator), BlueprintType, meta=(BlueprintSpawnableComponent, DisplayName="UiIndicator"))
class UISCREENFRAMEWORK_API UIndicatorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Deactivate() override;
	virtual void PostLoad() override;

	void AddIndicator(bool bMakeVisible = false);
	virtual void RemoveIndicator();
	const TObjectPtr<UBaseIndicatorViewModel>& GetIndicator() const { return IndicatorViewModel; }

	// Set visibility of indicator
	UFUNCTION(BlueprintCallable)
	virtual void SetIndicatorVisibility(bool bInVisibility, EIndicatorVisibilityPriority InPriority = EIndicatorVisibilityPriority::AlwaysAllowEnable);

	bool GetIndicatorVisibility() const { return IndicatorViewModel->GetIndicatorVisibility(); }

	TSoftClassPtr<UUserWidget> GetIndicatorClass() const { return IndicatorViewModel->GetIndicatorClass(); }
	TObjectPtr<UBaseIndicatorViewModel> GetIndicatorViewModel() const { return IndicatorViewModel; }

	void SetIndicatorClass(TSoftClassPtr<UUserWidget> InIndicatorWidgetClass);

	void SetIndicatorWorldOffset(const FVector& InWorldOffset) const;

	template <typename T>
	T* GetIndicatorViewModelByClass() const
	{
		return Cast<T>(IndicatorViewModel);
	}

protected:
	// View Model that keeps all data for the indicator
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UBaseIndicatorViewModel> IndicatorViewModel;

	// Indicates whether indicator should be added automatically on begin play
	UPROPERTY(EditAnywhere)
	bool bAddIndicatorOnBegin = true;

	// Indicates whether indicator should be visible automatically on begin play
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bAddIndicatorOnBegin"))
	bool bAutoVisible = false;

private:
	bool bAddedIndicator = false;
};
