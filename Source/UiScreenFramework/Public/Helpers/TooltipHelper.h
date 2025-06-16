#pragma once


struct FTooltipData;
class UOverlay;

namespace TooltipHelper
{
	FVector2D CalculateTooltipPosition(const FTooltipData& TooltipData);
	UOverlay* GetTooltipLayer(const UObject* WorldContextObject);
}
