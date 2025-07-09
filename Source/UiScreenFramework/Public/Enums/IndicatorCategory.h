// Copyright People Can Fly. All Rights Reserved."

#pragma once
#include "UObject/ObjectMacros.h"
#include "IndicatorCategory.generated.h"

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EIndicatorCategory : uint8
{
	None = 0 UMETA(Hidden),
	Default = 1 << 0,
	HealthBar = 1 << 1,
	Interaction = 1 << 2,
	InteractionMarker = 1 << 3,
	PickableItem = 1 << 4,
	DamageNumber = 1 << 5,

	All = Default | HealthBar | Interaction | InteractionMarker | PickableItem | DamageNumber,
};

ENUM_CLASS_FLAGS(EIndicatorCategory);
