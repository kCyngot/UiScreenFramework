#pragma once
#include "GameplayTagContainer.h"
#include "Enums/TooltipPlacement.h"

#include "TooltipData.generated.h"

class UBaseViewModel;

USTRUCT(BlueprintType)
struct FTooltipData
{
	GENERATED_BODY()

	/* Tooltip widget */
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> TooltipWidget;

	/* Tooltip widget class */
	UPROPERTY(Transient, BlueprintReadWrite)
	TSubclassOf<UUserWidget> TooltipWidgetClass;

	/* View model attached to the tooltip widget */
	UPROPERTY(Transient, BlueprintReadWrite)
	TObjectPtr<UBaseViewModel> TooltipViewModel;

	/* Geometry of a widget that tooltip is attached to */
	UPROPERTY(Transient, BlueprintReadWrite)
	FGeometry WidgetGeometry = FGeometry();

	/* Should the tooltip be on the right */
	UPROPERTY(Transient, BlueprintReadWrite)
	ETooltipPlacement TooltipPlacement = ETooltipPlacement::Left;

	/* Offset from position */
	UPROPERTY(Transient, BlueprintReadWrite)
	FVector2D TooltipOffset = FVector2D(0, 0);
};
