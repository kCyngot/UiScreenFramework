// Copyright People Can Fly. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Structs/ScreenEdgeMarkersTrackArea.h"
#include "Components/PanelWidget.h"
#include "UObject/ObjectMacros.h"
#include "Widgets/SWidget.h"
#include "IndicatorPanel.generated.h"

class SIndicatorCanvas;
/**
 * @class UIndicaorPanel
 * Widget component for displaying indicators
 */
UCLASS()
class UISCREENFRAMEWORK_API UIndicatorPanel : public UWidget
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Screen Edge Markers")
	FScreenEdgeMarkersTrackArea ScreenEdgeMarkersTrackArea;

	// UWidget interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End UWidget

	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);
	
protected:
	TSharedPtr<SIndicatorCanvas> MyActorCanvas;
};
