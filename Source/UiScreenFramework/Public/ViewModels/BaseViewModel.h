// Fill out your copyright notice in the Description page of Project Settings.

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
