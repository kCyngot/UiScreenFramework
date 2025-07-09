// Copyright People Can Fly. All Rights Reserved."

#pragma once

#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Structs/ScreenEdgeMarkersTrackArea.h"

class UCapsuleComponent;
class UBaseIndicatorViewModel;

namespace IndicatorProjectionHelper
{
	bool Project(const UBaseIndicatorViewModel& Indicator, const FSceneViewProjectionData& InProjectionData,
		const FVector2f& ScreenSize, const FScreenEdgeMarkersTrackArea& ScreenEdgeMarkersTrackArea, FVector2D& OutScreenPosition,
		bool& bOutIsOnTheTrack, float& OutTrackArrowAngle, FVector& OutWorldPosition);

	FBox GetBoundingBoxFromCapsule(UCapsuleComponent* Capsule);
	FBox GetBoundingBoxFromMesh(const USkeletalMeshComponent* MeshComponent);

	bool IsInsideScreenEdgeMarkerTrack(const FVector2D& ScreenPosition, const FVector2f& ScreenSize, const FScreenEdgeMarkersTrackArea& ScreenEdgeMarkersTrackArea);

	bool ProjectActorRoot(const FSceneViewProjectionData& InProjectionData, const FVector& ProjectWorldLocation, const FVector2f& ScreenSize, FVector2D& OutScreenSpacePosition);

	bool ProjectActorScreenBoundingBox(const FSceneViewProjectionData& InProjectionData, const FBox& BoundingBox, const FVector& BoundingBoxAnchor,
		const FVector2f& ScreenSize, FVector2D& OutScreenSpacePosition);

	bool ProjectActorBoundingBox(const FSceneViewProjectionData& InProjectionData, const FBox& BoundingBox, const FVector& BoundingBoxAnchor, const FVector& Center,
		const FVector& WorldPositionOffset, const FVector2f& ScreenSize, FVector2D& OutScreenSpacePosition);

	bool ProjectSkeletalMeshBoundingBox(const FSceneViewProjectionData& InProjectionData, const FBox& BoundingBox, const FVector& BoundingBoxAnchor, const FVector& Center,
		const FVector& WorldPositionOffset, const FVector2f& ScreenSize, FVector2D& OutScreenSpacePosition);
}
