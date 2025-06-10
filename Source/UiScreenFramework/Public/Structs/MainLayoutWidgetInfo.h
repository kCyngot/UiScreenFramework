#pragma once

#include "MainLayoutWidgetInfo.generated.h"

class UMainUiLayoutWidget;

USTRUCT()
struct FMainLayoutWidgetInfo
{
	GENERATED_BODY()
public:
	UPROPERTY(Transient)
	ULocalPlayer* LocalPlayer = nullptr;

	UPROPERTY(Transient)
	UMainUiLayoutWidget* MainLayoutWidget = nullptr;

	UPROPERTY(Transient)
	bool bAddedToViewport = false;

	FMainLayoutWidgetInfo()
	{
	}

	FMainLayoutWidgetInfo(ULocalPlayer* InLocalPlayer, UMainUiLayoutWidget* InMainLayoutWidget, const bool bIsInViewport)
		: LocalPlayer(InLocalPlayer)
		, MainLayoutWidget(InMainLayoutWidget)
		, bAddedToViewport(bIsInViewport)
	{
	}

	bool operator==(const ULocalPlayer* OtherLocalPlayer) const { return LocalPlayer == OtherLocalPlayer; }
};
