// Copyright People Can Fly. All Rights Reserved.

#include "ViewModels/BaseIndicatorViewModel.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Helpers/GeneralHelper.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BaseIndicatorViewModel)
DEFINE_LOG_CATEGORY(LogBaseIndicatorViewModel);

void UBaseIndicatorViewModel::SetIndicatorCategory(const EIndicatorCategory InIndicatorCategory)
{
	IndicatorCategory = InIndicatorCategory;
}

void UBaseIndicatorViewModel::SetIndicatorVisibility(bool bInVisibility, EIndicatorVisibilityPriority InPriority)
{
	// it allows to change visibility to true when priority is equal or higher than current one,
	// changing visibility to false is always possible
	if (!bInVisibility)
	{
		if (InPriority > CurrentVisibilityPriority)
		{
			CurrentVisibilityPriority = InPriority;
		}
		bVisibility = false;
	}
	else if (bInVisibility && InPriority >= CurrentVisibilityPriority)
	{
		CurrentVisibilityPriority = EIndicatorVisibilityPriority::AlwaysAllowEnable;
		bVisibility = true;
	}
}

void UBaseIndicatorViewModel::SetIndicatorClass(TSoftClassPtr<UUserWidget> InIndicatorWidgetClass)
{
	IndicatorWidgetClass = InIndicatorWidgetClass;
}

UUserWidget* UBaseIndicatorViewModel::GetIndicatorWidget()
{
	return IndicatorWidget.Get();
}

void UBaseIndicatorViewModel::SetProjectionMode(const EIndicatorProjectionMode InProjectionMode)
{
	ProjectionMode = InProjectionMode;
}

void UBaseIndicatorViewModel::SetHAlign(const EHorizontalAlignment InHAlignment)
{
	HAlignment = InHAlignment;
}

void UBaseIndicatorViewModel::SetVAlign(const EVerticalAlignment InVAlignment)
{
	VAlignment = InVAlignment;
}

void UBaseIndicatorViewModel::SetClampToScreen(const bool bValue)
{
	bClampToScreen = bValue;
}

void UBaseIndicatorViewModel::SetWorldPositionOffset(const FVector Offset)
{
	WorldPositionOffset = Offset;
}

void UBaseIndicatorViewModel::SetScreenSpaceOffset(const FVector2D Offset)
{
	ScreenSpaceOffset = Offset;
}

void UBaseIndicatorViewModel::SetBoundingBoxAnchor(const FVector InBoundingBoxAnchor)
{
	BoundingBoxAnchor = InBoundingBoxAnchor;
}

void UBaseIndicatorViewModel::SetTransitionTime(const float InTransitionTime)
{
	TransitionTime = InTransitionTime;
}

void UBaseIndicatorViewModel::SetHasVisibilityRange(const bool bValue)
{
	bHasVisibilityRange = bValue;
}

void UBaseIndicatorViewModel::SetShouldUpdateDistance(const bool bValue)
{
	bUpdateDistance = bValue;
}

void UBaseIndicatorViewModel::SetVisibilityRangeOuter(const float InVisibilityRangeOuter)
{
	VisibilityRangeOuter = InVisibilityRangeOuter;
}

void UBaseIndicatorViewModel::SetVisibilityRangeInner(const float InVisibilityRangeInner)
{
	VisibilityRangeInner = InVisibilityRangeInner;
}

void UBaseIndicatorViewModel::SetPriority(const int32 InPriority)
{
	Priority = InPriority;
}

bool UBaseIndicatorViewModel::IsPlayerWithinRange() const
{
	const APlayerController* PlayerController = GeneralHelper::GetPlayerController(this);
	const APawn* PlayerPawn = PlayerController ? PlayerController->GetPawn() : nullptr;
	const AActor* InActorAttachedTo = ActorAttachedTo.Get();
	if (PlayerPawn && InActorAttachedTo)
	{
		const float Distance = FVector::Dist(PlayerController->GetPawn()->GetActorLocation(), InActorAttachedTo->GetActorLocation());
		if (Distance > VisibilityRangeOuter)
		{
			return false;
		}
		
		return true;
	}
	return false;
}

float UBaseIndicatorViewModel::CalculateOuterDistanceFactor(const float Distance, const float OuterRange, float InnerRange)
{
	if (Distance > OuterRange)
	{
		return 0.f;
	}
	if (Distance <= InnerRange)
	{
		return 1.f;
	}
	const float DistanceFromOuterToInnerRange = OuterRange - InnerRange;

	// The DistanceFactor adjusts linearly: 0 when the player is at the VisibilityRangeOuter radius and 1 when at the VisibilityRangeInner radius.
	const float OutDistanceFactor = (DistanceFromOuterToInnerRange <= 0) ? 1.f : (1.f - (Distance - InnerRange) / DistanceFromOuterToInnerRange);

	UE_LOG(LogBaseIndicatorViewModel, VeryVerbose, TEXT("%hs Distance: %f, DistanceFromOuterToInnerRange %f, OutDistanceFactor %f, VisibilityRangeInner %f, VisibilityRangeOuter %f"),
		__FUNCTION__, Distance, DistanceFromOuterToInnerRange, OutDistanceFactor, InnerRange, OuterRange);

	return OutDistanceFactor;
}

float UBaseIndicatorViewModel::GetDistanceFactor() const
{
	const APlayerController* PlayerController = GeneralHelper::GetPlayerController(this);
	const APawn* PlayerPawn = PlayerController ? PlayerController->GetPawn() : nullptr;
	const AActor* InActorAttachedTo = ActorAttachedTo.Get();
	if (!PlayerPawn || !InActorAttachedTo)
	{
		return 0.f;
	}

	const float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), InActorAttachedTo->GetActorLocation());

	return CalculateOuterDistanceFactor(Distance, VisibilityRangeOuter, VisibilityRangeInner);
}

void UBaseIndicatorViewModel::ResetActorAttachedTo()
{
	ActorAttachedTo.Reset();
}

void UBaseIndicatorViewModel::SetActorAttachedTo(AActor* InActorAttachedTo)
{
	if (InActorAttachedTo)
	{
		ActorAttachedTo = InActorAttachedTo;
	}
}

void UBaseIndicatorViewModel::SetFixedWorldPosition(const FVector& InFixedWorldPosition)
{
	FixedWorldPosition = InFixedWorldPosition;
}

void UBaseIndicatorViewModel::HandleVisibilityCheck()
{
	if (IsPlayerWithinRange())
	{
		SetIndicatorVisibility(true);
	}
	else
	{
		SetIndicatorVisibility(false);
	}
}

void UBaseIndicatorViewModel::UpdateDistanceFactor()
{
	//TODO implement using view model
}