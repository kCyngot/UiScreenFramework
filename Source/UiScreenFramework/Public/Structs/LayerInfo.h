#pragma once
#include "CommonLazyWidget.h"
#include "GameplayTagContainer.h"

#include "LayerInfo.generated.h"


USTRUCT(BlueprintType)
struct FLayerInfo
{
	GENERATED_BODY()

	// The unique tag that identifies this layer.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "UI.Layer"))
	FGameplayTag LayerId;

	// The lazy widget associated with this layer.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCommonLazyWidget> LayerWidget;
};
