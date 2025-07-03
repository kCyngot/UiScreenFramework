// Copyright People Can Fly. All Rights Reserved."

#pragma once
#include "GameplayTagContainer.h"

#include "LayerInfo.generated.h"

class ULayerWidget;

USTRUCT(BlueprintType)
struct FLayerInfo
{
	GENERATED_BODY()

	/* The unique tag that identifies this layer. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "UI.Layer"))
	FGameplayTag LayerId;

	/* The lazy widget associated with this layer. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<ULayerWidget> LayerWidget;
};
