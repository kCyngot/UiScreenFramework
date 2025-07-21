// Copyright People Can Fly. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Layout/Margin.h"

#include "ScreenEdgeMarkersTrackArea.generated.h"

/**
 * Data to describe the Screen Edge Markers Track Area
 */
USTRUCT(BlueprintType)
struct UISCREENFRAMEWORK_API FScreenEdgeMarkersTrackArea
{
	GENERATED_BODY()

public:
	// Offsets that will be subtracted from the indicator canvas dimensions to obtain the clamping area.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FMargin Offsets = FMargin(40.f);
};