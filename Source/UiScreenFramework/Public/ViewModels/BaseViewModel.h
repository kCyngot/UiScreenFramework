// Copyright People Can Fly. All Rights Reserved."

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "BaseViewModel.generated.h"


UCLASS(Abstract)
class UISCREENFRAMEWORK_API UBaseViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	virtual void Deinit()
	{
	};
};
