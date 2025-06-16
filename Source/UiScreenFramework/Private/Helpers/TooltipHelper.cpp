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
	if (!TooltipWidget)
	{
		return FVector2D::Zero();
	}

	TooltipWidget->ForceLayoutPrepass();
	UWorld* World = TooltipWidget->GetWorld();
	FVector2D PixelPos, ViewportPos;
	FGeometry WidgetGeometry = TooltipData.WidgetGeometry;
	
	USlateBlueprintLibrary::LocalToViewport(World, WidgetGeometry, FVector2D::Zero(), PixelPos, ViewportPos);

	const FGeometry Viewport = UWidgetLayoutLibrary::GetViewportWidgetGeometry(World);
	const FVector2D ViewportSize = Viewport.GetLocalSize();
	const FVector2D TooltipSize = TooltipWidget->GetDesiredSize();

	//Fixed pos
	if (TooltipData.bUseFixedPoint)
	{
		const FVector2D TooltipHalfSize = TooltipSize * 0.5f;
		return (ViewportSize * TooltipData.FixedPoint) - TooltipHalfSize;
	}

	FVector2D FinalPos;
	const FVector2D WidgetLocalSizeX = FVector2D(WidgetGeometry.GetLocalSize().X, 0);
	const FVector2D TooltipDesiredSizeX = FVector2D(TooltipSize.X, 0);

	if (TooltipData.TooltipPlacement == ETooltipPlacement::Right)
	{
		FinalPos = ViewportPos + WidgetLocalSizeX + TooltipData.TooltipOffset;

		// Setting tooltip to display on left if offscreen
		if (FinalPos.X + TooltipSize.X > ViewportSize.X)
		{
			FinalPos = ViewportPos - TooltipDesiredSizeX - TooltipData.TooltipOffset;
		}
	}
	else
	{
		FinalPos = ViewportPos - TooltipDesiredSizeX - TooltipData.TooltipOffset;

		// Setting tooltip to display on right if offscreen
		if (FinalPos.X < 0)
		{
			FinalPos = ViewportPos + WidgetLocalSizeX + TooltipData.TooltipOffset;
		}
	}

	// Check tooltip is on screen Y
	const UUiScreenFrameworkSettings& ScreenFrameworkSettings = UiScreenManagerHelper::GetUiScreenFrameworkSettings();
	const float TooltipEdgePadding = ScreenFrameworkSettings.GetTooltipEdgePadding();
	if (FinalPos.Y + TooltipSize.Y > ViewportSize.Y)
	{
		FinalPos.Y = ViewportSize.Y - TooltipSize.Y - TooltipEdgePadding;
	}
	else if (FinalPos.Y < 0)
	{
		FinalPos.Y = TooltipEdgePadding;
	}

	return FinalPos;
}

UOverlay* TooltipHelper::GetTooltipLayer(const UObject* WorldContextObject)
{
	const UUiScreenManager& ScreenManager = UUiScreenManager::GetChecked(WorldContextObject);
	const UMainUiLayoutWidget* MainUiLayoutWidget = ScreenManager.GetMainLayoutWidgetInfo().MainLayoutWidget;

	return MainUiLayoutWidget->GetTooltipLayer();
}
