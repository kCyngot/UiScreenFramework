// Copyright People Can Fly. All Rights Reserved."

#pragma once
#include "Engine/LocalPlayer.h"
#include "MainLayoutWidgetInfo.generated.h"

class UMainUiLayoutWidget;

USTRUCT()
struct FMainLayoutWidgetInfo
{
	GENERATED_BODY()
public:
	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> LocalPlayer;

	UPROPERTY(Transient)
	TObjectPtr<UMainUiLayoutWidget> MainLayoutWidget;

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
