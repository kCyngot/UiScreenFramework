// Copyright People Can Fly. All Rights Reserved."

#pragma once
#include "GameplayTagContainer.h"
#include "Enums/TooltipPlacement.h"

#include "TooltipPositionData.generated.h"

USTRUCT(BlueprintType)
struct FTooltipPositionData
{
	GENERATED_BODY()

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
};