// Copyright People Can Fly. All Rights Reserved."

#include "Helpers/TooltipHelper.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "DeveloperSettings/UiScreenFrameworkSettings.h"
#include "Helpers/UiScreenManagerHelper.h"
#include "Structs/TooltipData.h"
#include "Subsystems/UiScreenManager.h"

FVector2D TooltipHelper::CalculateTooltipPosition(const FTooltipData& TooltipData)
{
	const TObjectPtr<UUserWidget> TooltipWidget = TooltipData.TooltipWidget;
	if (!ensure(TooltipWidget))
	{
		return FVector2D::Zero();
	}

	TooltipWidget->ForceLayoutPrepass();

	UWorld* World = TooltipWidget->GetWorld();
	if (!ensure(World))
	{
		return FVector2D::Zero();
	}

	const FVector2D TooltipSize = TooltipWidget->GetDesiredSize();
	const FGeometry ViewportGeometry = UWidgetLayoutLibrary::GetViewportWidgetGeometry(World);
	const FVector2D ViewportSize = ViewportGeometry.GetLocalSize();

	FVector2D PixelPosition, ViewportPosition;
	USlateBlueprintLibrary::LocalToViewport(World, TooltipData.WidgetGeometry, FVector2D::Zero(), PixelPosition, ViewportPosition);

	FVector2D FinalPosition = ViewportPosition;
	const float ParentWidgetWidth = TooltipData.WidgetGeometry.GetLocalSize().X;

	const UUiScreenFrameworkSettings& ScreenFrameworkSettings = UiScreenManagerHelper::GetUiScreenFrameworkSettings();
	const float TooltipEdgePadding = ScreenFrameworkSettings.GetTooltipEdgePadding();
	
	const float PositionRightX = ViewportPosition.X + ParentWidgetWidth + TooltipData.TooltipOffset.X;
	const float PositionLeftX = ViewportPosition.X - TooltipSize.X - TooltipData.TooltipOffset.X;

	bool bPlaceOnRight = (TooltipData.TooltipPlacement == ETooltipPlacement::Right);

	if (bPlaceOnRight)
	{
		if (PositionRightX + TooltipSize.X + TooltipEdgePadding > ViewportSize.X)
		{
			bPlaceOnRight = false;
		}
	}
	else
	{
		if (PositionLeftX < TooltipEdgePadding)
		{
			bPlaceOnRight = true;
		}
	}

	FinalPosition.X = bPlaceOnRight ? PositionRightX : PositionLeftX;
	FinalPosition.Y += TooltipData.TooltipOffset.Y;

	const float MaximumVerticalPosition = ViewportSize.Y - TooltipSize.Y - TooltipEdgePadding;
	FinalPosition.Y = FMath::Clamp(FinalPosition.Y, TooltipEdgePadding, MaximumVerticalPosition);

	const float MaximumHorizontalPosition = ViewportSize.X - TooltipSize.X - TooltipEdgePadding;
	FinalPosition.X = FMath::Clamp(FinalPosition.X, TooltipEdgePadding, MaximumHorizontalPosition);

	return FinalPosition;
}

UOverlay* TooltipHelper::GetTooltipLayer(const UObject* WorldContextObject)
{
	const UUiScreenManager& ScreenManager = UUiScreenManager::GetChecked(WorldContextObject);
	const UMainUiLayoutWidget* MainUiLayoutWidget = ScreenManager.GetMainLayoutWidgetInfo().MainLayoutWidget;

	return MainUiLayoutWidget->GetTooltipLayer();
}
