// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Structs/UiScreenInfo.h"
#include "UiScreensData.generated.h"


UCLASS(BlueprintType)
class UISCREENFRAMEWORK_API UUiScreensData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, meta = (TitleProperty = "ScreenId"))
	TArray<FUiScreenInfo> Screens;
};
