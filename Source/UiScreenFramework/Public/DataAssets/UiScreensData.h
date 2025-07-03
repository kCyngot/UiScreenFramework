// Copyright People Can Fly. All Rights Reserved."

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
