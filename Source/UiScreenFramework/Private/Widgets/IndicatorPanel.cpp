// Copyright People Can Fly. All Rights Reserved.

#include "Widgets/IndicatorPanel.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Layout/SBox.h"
#include "Engine/LocalPlayer.h"
#include "Widgets/SIndicatorCanvas.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(IndicatorPanel)

UIndicatorPanel::UIndicatorPanel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsVariable = true;
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UIndicatorPanel::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyActorCanvas.Reset();
}

TSharedRef<SWidget> UIndicatorPanel::RebuildWidget()
{
	if (!IsDesignTime())
	{
		ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
		if (ensureMsgf(LocalPlayer, TEXT("Attempting to rebuild a UActorCanvas without a valid LocalPlayer!")))
		{
			MyActorCanvas = SNew(SIndicatorCanvas, FLocalPlayerContext(LocalPlayer), ScreenEdgeMarkersTrackArea);
		
			return MyActorCanvas.ToSharedRef();
		}
	}

	// Give it a trivial box, NullWidget isn't safe to use from a UWidget
	return SNew(SBox);
}

void UIndicatorPanel::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	Super::AddReferencedObjects(InThis, Collector);
	UIndicatorPanel* This = CastChecked<UIndicatorPanel>(InThis);
	if (This->MyActorCanvas.IsValid())
	{
		This->MyActorCanvas->AddReferencedObjects(Collector);
	}
}

