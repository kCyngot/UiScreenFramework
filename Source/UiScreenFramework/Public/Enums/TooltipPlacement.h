#pragma once
#include "UObject/ObjectMacros.h"
#include "TooltipPlacement.generated.h"

UENUM(BlueprintType)
enum class ETooltipPlacement : uint8
{
	Left,
	Right,
};