// Copyright People Can Fly. All Rights Reserved."

#pragma once
#include "UObject/ObjectMacros.h"
#include "IndicatorProjectionMode.generated.h"

UENUM(BlueprintType)
enum class EIndicatorProjectionMode : uint8
{
	FixedPoint,
	ActorRoot,
	ActorBoundingBox,
	ActorSkeletalMeshBoundingBox,
	ActorScreenBoundingBox,
};
