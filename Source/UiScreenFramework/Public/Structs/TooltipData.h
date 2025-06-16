#pragma once
#include "GameplayTagContainer.h"
#include "Enums/TooltipPlacement.h"

#include "TooltipData.generated.h"

class UBaseViewModel;

USTRUCT(BlueprintType)
struct FTooltipData
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FGameplayTag TooltipTag = FGameplayTag::EmptyTag;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> TooltipWidget;

	UPROPERTY(Transient, BlueprintReadWrite)
	TSubclassOf<UUserWidget> TooltipWidgetClass;

	UPROPERTY(Transient, BlueprintReadWrite)
	TObjectPtr<UBaseViewModel> TooltipViewModel;

	/* Geometry to attach to */
	UPROPERTY(Transient, BlueprintReadWrite)
	FGeometry WidgetGeometry = FGeometry();

	/* Should the tooltip be on the right */
	UPROPERTY(Transient, BlueprintReadWrite)
	ETooltipPlacement TooltipPlacement = ETooltipPlacement::Left;

	/* Should a fixed point be used */
	UPROPERTY(Transient, BlueprintReadWrite)
	bool bUseFixedPoint = false;

	/* Offset from position */
	UPROPERTY(Transient, BlueprintReadWrite)
	FVector2D TooltipOffset = FVector2D(0, 0);

	/* Fixed position of tooltip */
	UPROPERTY(Transient, BlueprintReadWrite)
	FVector2D FixedPoint = FVector2D(0, 0);

	FTooltipData() = default;

	FTooltipData(const FGameplayTag InTooltipTag, const TObjectPtr<UUserWidget>& InTooltipWidget)
		: TooltipTag(InTooltipTag)
		  , TooltipWidget(InTooltipWidget)
	{
	}
};
