// Copyright People Can Fly. All Rights Reserved."

#pragma once

#include "CoreMinimal.h"
#include "BaseViewModel.h"
#include "UObject/Object.h"
#include "ScreenViewModel.generated.h"


UCLASS(Abstract)
class UISCREENFRAMEWORK_API UScreenViewModel : public UBaseViewModel
{
	GENERATED_BODY()

public:
	virtual void Init();
	virtual void Deinit() override;

private:
	bool bHasBeenInit = false;
};
