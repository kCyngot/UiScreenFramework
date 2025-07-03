// Copyright People Can Fly. All Rights Reserved."

#pragma once
#include "Math/Vector2D.h"


struct FTooltipData;
class UOverlay;

namespace TooltipHelper
{
	FVector2D CalculateTooltipPosition(const FTooltipData& TooltipData);
	UOverlay* GetTooltipLayer(const UObject* WorldContextObject);
}
