// Fill out your copyright notice in the Description page of Project Settings.

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
