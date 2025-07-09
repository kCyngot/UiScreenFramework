// Copyright People Can Fly. All Rights Reserved."

#pragma once
#include "UObject/ObjectMacros.h"
#include "IndicatorVisibilityPriority.generated.h"

UENUM(BlueprintType)
enum class EIndicatorVisibilityPriority : uint8
{
	AlwaysAllowEnable,
	GameplayCondition,
};