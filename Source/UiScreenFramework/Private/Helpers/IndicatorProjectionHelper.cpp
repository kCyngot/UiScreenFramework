// Copyright People Can Fly. All Rights Reserved."

#include "Helpers/IndicatorProjectionHelper.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Enums/IndicatorProjectionMode.h"
#include "GameFramework/Character.h"
#include "ViewModels/BaseIndicatorViewModel.h"


namespace IndicatorProjectionHelper
{
	bool Project(const UBaseIndicatorViewModel& Indicator, const FSceneViewProjectionData& InProjectionData, const FVector2f& ScreenSize, const FScreenEdgeMarkersTrackArea& ScreenEdgeMarkersTrackArea,
		FVector2D& OutScreenPosition, bool& bOutIsOnTheTrack, float& OutTrackArrowAngle, FVector& OutWorldPosition)
	{
		const EIndicatorProjectionMode ProjectionMode = Indicator.GetProjectionMode();

		if (ProjectionMode != EIndicatorProjectionMode::FixedPoint)
		{
			const AActor* ActorAttachedTo = Indicator.GetActorAttachedTo();
			if (!IsValid(ActorAttachedTo))
			{
				UE_LOG(LogBaseIndicatorViewModel, Warning, TEXT("%hs: ActorAttachedTo isn't set"), __FUNCTION__);
				return false;
			}
		}

		FBox BoundingBox;
		FVector Center = FVector::Zero();

		const ACharacter* CharacterTypeActor = Cast<ACharacter>(Indicator.GetActorAttachedTo());
		if (IsValid(CharacterTypeActor))
		{
			if (ProjectionMode == EIndicatorProjectionMode::ActorSkeletalMeshBoundingBox)
			{
				BoundingBox = GetBoundingBoxFromMesh(CharacterTypeActor->GetMesh());
				const FVector ActorLocation = CharacterTypeActor->GetActorLocation();
				Center = FVector(ActorLocation.X, ActorLocation.Y, BoundingBox.GetCenter().Z);
			}
			else
			{
				BoundingBox = GetBoundingBoxFromCapsule(CharacterTypeActor->GetCapsuleComponent());
				Center = BoundingBox.GetCenter();
			}
		}
		else if (ProjectionMode != EIndicatorProjectionMode::FixedPoint)
		{
			const AActor* ActorAttachedTo = Indicator.GetActorAttachedTo();
			BoundingBox = ActorAttachedTo->GetComponentsBoundingBox();

			USceneComponent* RootComponent = ActorAttachedTo->GetRootComponent();
			if (!IsValid(RootComponent))
			{
				UE_LOG(LogBaseIndicatorViewModel, Log, TEXT("%s Root actor with not found attached Actor - %s "), *FString(__FUNCTION__), *ActorAttachedTo->GetName());
			}

			if (RootComponent && (ProjectionMode == EIndicatorProjectionMode::ActorRoot))
			{
				Center = RootComponent->GetComponentLocation();
			}
			else
			{
				Center = BoundingBox.GetCenter();
			}
		}

		const bool bClampToScreen = Indicator.GetClampToScreen();

		bool bWasProjectionOk;

		switch (ProjectionMode)
		{
		case EIndicatorProjectionMode::FixedPoint:
			OutWorldPosition = Indicator.GetFixedWorldPosition() + Indicator.GetWorldPositionOffset();
			bWasProjectionOk = ProjectActorRoot(InProjectionData, OutWorldPosition, ScreenSize, OutScreenPosition);
			break;
		case EIndicatorProjectionMode::ActorRoot:
			OutWorldPosition = Center + Indicator.GetWorldPositionOffset();
			bWasProjectionOk = ProjectActorRoot(InProjectionData, OutWorldPosition, ScreenSize, OutScreenPosition);
			break;
		case EIndicatorProjectionMode::ActorBoundingBox:
			OutWorldPosition = Center;
			bWasProjectionOk = ProjectActorBoundingBox(InProjectionData, BoundingBox, Indicator.GetBoundingBoxAnchor(), OutWorldPosition, Indicator.GetWorldPositionOffset(), ScreenSize,
				OutScreenPosition);
			break;
		case EIndicatorProjectionMode::ActorSkeletalMeshBoundingBox:
			OutWorldPosition = Center;
			bWasProjectionOk = ProjectSkeletalMeshBoundingBox(InProjectionData, BoundingBox, Indicator.GetBoundingBoxAnchor(), OutWorldPosition, Indicator.GetWorldPositionOffset(), ScreenSize,
				OutScreenPosition);
			break;
		case EIndicatorProjectionMode::ActorScreenBoundingBox:
			OutWorldPosition = BoundingBox.GetCenter();
			bWasProjectionOk = ProjectActorScreenBoundingBox(InProjectionData, BoundingBox, Indicator.GetBoundingBoxAnchor(), ScreenSize, OutScreenPosition);
			break;
		default:
			check(false);
			return false;
		}

		OutScreenPosition += Indicator.GetScreenSpaceOffset();

		if (!bClampToScreen)
		{
			bOutIsOnTheTrack = false;
			return bWasProjectionOk;
		}

		if (bWasProjectionOk)
		{
			// bClampToScreen is true, but we don't need to clamp if it is inside.
			if (IsInsideScreenEdgeMarkerTrack(OutScreenPosition, ScreenSize, ScreenEdgeMarkersTrackArea))
			{
				bOutIsOnTheTrack = false;
				return true;
			}
		}

		// Clamp screen edge marker into area track.
		bOutIsOnTheTrack = true;

		const FVector2D HalfScreenSize = FVector2D(ScreenSize) * 0.5;
		FVector2D CartesianCoords = OutScreenPosition - HalfScreenSize;
		const FVector2D MarkerAreaHalfDimensions = HalfScreenSize - FVector2D(ScreenEdgeMarkersTrackArea.Offsets.Left, ScreenEdgeMarkersTrackArea.Offsets.Top);

		double RatioX = UE_BIG_NUMBER;
		if (!FMath::IsNearlyZero(CartesianCoords.X))
		{
			const double ProjectedXAbs = FMath::Abs(CartesianCoords.X);
			RatioX = MarkerAreaHalfDimensions.X / ProjectedXAbs;
		}

		double RatioY = UE_BIG_NUMBER;
		if (!FMath::IsNearlyZero(CartesianCoords.Y))
		{
			const double ProjectedYAbs = FMath::Abs(CartesianCoords.Y);
			RatioY = MarkerAreaHalfDimensions.Y / ProjectedYAbs;
		}

		double MinRatio;

		if (RatioX <= RatioY)
		{
			MinRatio = RatioX;
			OutTrackArrowAngle = (CartesianCoords.X > 0.0) ? 0.f : 180.f;
		}
		else
		{
			MinRatio = RatioY;
			OutTrackArrowAngle = (CartesianCoords.Y > 0.0) ? 90.f : -90.f;
		}

		// CartesianCoords on the track
		CartesianCoords *= MinRatio;

		// Return to screen space
		OutScreenPosition = CartesianCoords + HalfScreenSize;

		return true;
	}

	FBox GetBoundingBoxFromCapsule(UCapsuleComponent* Capsule)
	{
		if (!Capsule)
		{
			return FBox();
		}

		const FVector Origin = Capsule->GetComponentLocation();
		const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		const float Radius = Capsule->GetScaledCapsuleRadius();

		const FVector Min = Origin - FVector(Radius, Radius, HalfHeight);
		const FVector Max = Origin + FVector(Radius, Radius, HalfHeight);

		return FBox(Min, Max);
	}

	FBox GetBoundingBoxFromMesh(const USkeletalMeshComponent* MeshComponent)
	{
		if (MeshComponent)
		{
			const FBoxSphereBounds Bounds = MeshComponent->CalcBounds(MeshComponent->GetComponentTransform());
			return Bounds.GetBox();
		}
		return FBox(nullptr, 0);
	}

	bool IsInsideScreenEdgeMarkerTrack(const FVector2D& ScreenPosition, const FVector2f& ScreenSize, const FScreenEdgeMarkersTrackArea& ScreenEdgeMarkersTrackArea)
	{
		const FMargin& Offsets = ScreenEdgeMarkersTrackArea.Offsets;

		return ((ScreenPosition.X >= Offsets.Left) && (ScreenPosition.X <= (ScreenSize.X - Offsets.Right)) &&
			(ScreenPosition.Y >= Offsets.Top) && (ScreenPosition.Y <= (ScreenSize.Y - Offsets.Bottom)));
	}

	bool ProjectActorRoot(const FSceneViewProjectionData& InProjectionData, const FVector& ProjectWorldLocation, const FVector2f& ScreenSize, FVector2D& OutScreenSpacePosition)
	{
		return ULocalPlayer::GetPixelPoint(InProjectionData, ProjectWorldLocation, OutScreenSpacePosition, &ScreenSize);
	}

	bool ProjectActorScreenBoundingBox(const FSceneViewProjectionData& InProjectionData, const FBox& BoundingBox, const FVector& BoundingBoxAnchor, const FVector2f& ScreenSize,
		FVector2D& OutScreenSpacePosition)
	{
		FVector2D LowerLeft, UpperRight;
		if (ULocalPlayer::GetPixelBoundingBox(InProjectionData, BoundingBox, LowerLeft, UpperRight, &ScreenSize))
		{
			OutScreenSpacePosition.X = FMath::Lerp(LowerLeft.X, UpperRight.X, BoundingBoxAnchor.X);
			OutScreenSpacePosition.Y = FMath::Lerp(LowerLeft.Y, UpperRight.Y, BoundingBoxAnchor.Y);
			return true;
		}

		return false;
	}

	bool ProjectActorBoundingBox(const FSceneViewProjectionData& InProjectionData, const FBox& BoundingBox, const FVector& BoundingBoxAnchor, const FVector& Center, const FVector& WorldPositionOffset,
		const FVector2f& ScreenSize, FVector2D& OutScreenSpacePosition)
	{
		const FVector ProjectBoxPoint = Center + (BoundingBox.GetSize() * (BoundingBoxAnchor - FVector(0.5))) + WorldPositionOffset;

		return ULocalPlayer::GetPixelPoint(InProjectionData, ProjectBoxPoint, OutScreenSpacePosition, &ScreenSize);
	}

	bool ProjectSkeletalMeshBoundingBox(const FSceneViewProjectionData& InProjectionData, const FBox& BoundingBox, const FVector& BoundingBoxAnchor, const FVector& Center,
		const FVector& WorldPositionOffset, const FVector2f& ScreenSize, FVector2D& OutScreenSpacePosition)
	{
		return ProjectActorBoundingBox(InProjectionData, BoundingBox, BoundingBoxAnchor, Center, WorldPositionOffset, ScreenSize, OutScreenSpacePosition);
	}
}
