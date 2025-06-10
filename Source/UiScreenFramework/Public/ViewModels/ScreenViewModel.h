// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "UObject/Object.h"
#include "ScreenViewModel.generated.h"


UCLASS(Abstract)
class UISCREENFRAMEWORK_API UScreenViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	virtual void Init();
	virtual void Deinit();

protected:
	virtual void SetBindings(const bool bShouldBind)
	{
	};

	virtual void CleanAllPointers()
	{
	};

private:
	bool bHasBeenInit = false;
};
