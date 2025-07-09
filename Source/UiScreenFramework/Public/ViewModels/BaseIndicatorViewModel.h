// Copyright People Can Fly. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseViewModel.h"
#include "Enums/IndicatorCategory.h"
#include "Enums/IndicatorProjectionMode.h"
#include "Enums/IndicatorVisibilityPriority.h"
#include "Types/SlateEnums.h"
#include "BaseIndicatorViewModel.generated.h"

class SWidget;
class UUserWidget;

DECLARE_LOG_CATEGORY_EXTERN(LogBaseIndicatorViewModel, Log, All);

UCLASS(Blueprintable, EditInlineNew)
class UISCREENFRAMEWORK_API UBaseIndicatorViewModel : public UBaseViewModel
{
	GENERATED_BODY()

public:
	void SetIsIndicatorClamped(const bool bInIsIndicatorClamped) { UE_MVVM_SET_PROPERTY_VALUE(bIsIndicatorClamped, bInIsIndicatorClamped); }
	void SetClampAngle(const float InClampAngle) { UE_MVVM_SET_PROPERTY_VALUE(ClampAngle, InClampAngle); }

protected:
	// Indicator category identifier
	UPROPERTY(EditAnywhere)
	EIndicatorCategory IndicatorCategory = EIndicatorCategory::Default;

	// Indicator user widget class
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UUserWidget> IndicatorWidgetClass;

	// Indicator's Projection Mode on the screen
	UPROPERTY(EditAnywhere)
	EIndicatorProjectionMode ProjectionMode = EIndicatorProjectionMode::ActorBoundingBox;

	// Horizontal alignment 
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EHorizontalAlignment> HAlignment = HAlign_Center;

	// Vertical alignment 
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EVerticalAlignment> VAlignment = VAlign_Center;

	// Anchor point for indicator that is located on its owner bounding box (default value is top of bounding box)
	UPROPERTY(EditAnywhere)
	FVector BoundingBoxAnchor = FVector(0.5f, 0.5f, 1.0f);

	// Additional offset on screen space
	UPROPERTY(EditAnywhere)
	FVector2D ScreenSpaceOffset = FVector2D(0.0f, 0.0f);

	// Additional Offset in World (default value make indicator move up a little bit)
	UPROPERTY(EditAnywhere)
	FVector WorldPositionOffset = FVector(0.0f, 0.0f, 20.0f);

	// Time of transition between indicator's visibility modes
	UPROPERTY(EditAnywhere)
	float TransitionTime = .2f;

	// Should indicator stay on screen if its owner is out of screen
	UPROPERTY(EditAnywhere)
	bool bClampToScreen = false;

	// Indicates whether indicator should update the distance factor
	UPROPERTY(EditAnywhere)
	bool bUpdateDistance = false;

	// Indicates whether indicator has distance visibility condition
	UPROPERTY(EditAnywhere)
	bool bHasVisibilityRange = false;

	// Indicates range when player starts seeing indicator
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bHasVisibilityRange || bUpdateDistance"))
	float VisibilityRangeOuter = 1500.0f;

	// Indicates range when indicator is fully visible
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bHasVisibilityRange || bUpdateDistance"))
	float VisibilityRangeInner = 1000.0f;

	// Indicates range when indicator is fully visible
	UPROPERTY(BlueprintReadOnly, FieldNotify)
	bool bIsIndicatorClamped = false;

	// Indicates range when indicator is fully visible
	UPROPERTY(BlueprintReadOnly, FieldNotify)
	float ClampAngle = 0;

public:
	static float CalculateOuterDistanceFactor(float Distance, float OuterRange, float InnerRange);

	EIndicatorCategory GetIndicatorCategory() const { return IndicatorCategory; }

	void SetIndicatorCategory(EIndicatorCategory InIndicatorCategory);

	bool GetIndicatorVisibility() const { return bVisibility; }

	void SetIndicatorVisibility(bool bInVisibility, EIndicatorVisibilityPriority InPriority = EIndicatorVisibilityPriority::AlwaysAllowEnable);

	TSoftClassPtr<UUserWidget> GetIndicatorClass() const { return IndicatorWidgetClass; }

	void SetIndicatorClass(TSoftClassPtr<UUserWidget> InIndicatorWidgetClass);

	virtual UUserWidget* GetIndicatorWidget();

	TWeakObjectPtr<UUserWidget> IndicatorWidget;

	// Layout Properties
	//=======================

	EIndicatorProjectionMode GetProjectionMode() const { return ProjectionMode; }

	void SetProjectionMode(EIndicatorProjectionMode InProjectionMode);

	// Horizontal alignment to the point in space to place the indicator at.
	EHorizontalAlignment GetHAlign() const { return HAlignment; }

	void SetHAlign(EHorizontalAlignment InHAlignment);

	// Vertical alignment to the point in space to place the indicator at.
	EVerticalAlignment GetVAlign() const { return VAlignment; }

	void SetVAlign(EVerticalAlignment InVAlignment);

	// Clamp the indicator to the edge of the screen?
	bool GetClampToScreen() const { return bClampToScreen; }

	void SetClampToScreen(bool bValue);

	// The position offset for the indicator in world space.
	FVector GetWorldPositionOffset() const { return WorldPositionOffset; }

	void SetWorldPositionOffset(const FVector Offset);

	// The position offset for the indicator in screen space.
	FVector2D GetScreenSpaceOffset() const { return ScreenSpaceOffset; }

	void SetScreenSpaceOffset(const FVector2D Offset);

	FVector GetBoundingBoxAnchor() const { return BoundingBoxAnchor; }

	void SetBoundingBoxAnchor(const FVector InBoundingBoxAnchor);

	float GetTransitionTime() const { return TransitionTime; }

	void SetTransitionTime(float InTransitionTime);

	bool GetHasVisibilityRange() const { return bHasVisibilityRange; }

	void SetHasVisibilityRange(bool bValue);

	bool ShouldUpdateDistance() const { return bUpdateDistance; }

	void SetShouldUpdateDistance(const bool bValue);

	float GetVisibilityRangeOuter() const { return VisibilityRangeOuter; }

	void SetVisibilityRangeOuter(float InVisibilityRangeOuter);

	float GetVisibilityRangeInner() const { return VisibilityRangeInner; }

	void SetVisibilityRangeInner(float InVisibilityRangeInner);

	// Sorting Properties
	//=======================

	// Allows sorting the indicators (after they are sorted by depth), to allow some group of indicators
	// to always be in front of others.
	int32 GetPriority() const { return Priority; }

	void SetPriority(int32 InPriority);

	AActor* GetActorAttachedTo() const { return ActorAttachedTo.Get(); }
	void ResetActorAttachedTo();

	// Set Actor that indicator is attached to
	UFUNCTION(BlueprintCallable)
	void SetActorAttachedTo(AActor* InActorAttachedTo);

	FVector GetFixedWorldPosition() const { return FixedWorldPosition; }
	void SetFixedWorldPosition(const FVector& InFixedWorldPosition);

	void HandleVisibilityCheck();
	void UpdateDistanceFactor();

private:
	bool IsPlayerWithinRange() const;
	float GetDistanceFactor() const;
	bool bVisibility = false;

	bool bAutoRemoveWhenIndicatorComponentIsNull = true;

	int32 Priority = 0;

	friend class SIndicatorCanvas;

	TWeakObjectPtr<AActor> ActorAttachedTo = nullptr;

	UPROPERTY(Transient)
	FVector FixedWorldPosition = FVector::ZeroVector;

	TWeakPtr<SWidget> CanvasHost;

	EIndicatorVisibilityPriority CurrentVisibilityPriority = EIndicatorVisibilityPriority::AlwaysAllowEnable;
};
