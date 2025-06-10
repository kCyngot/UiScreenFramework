﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "View/MVVMViewModelContextResolver.h"
#include "ScreenViewModelResolver.generated.h"


UCLASS()
class UISCREENFRAMEWORK_API UScreenViewModelResolver : public UMVVMViewModelContextResolver
{
	GENERATED_BODY()
public:
	virtual UObject* CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const override;
};
