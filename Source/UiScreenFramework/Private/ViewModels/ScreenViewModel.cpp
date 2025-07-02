// Fill out your copyright notice in the Description page of Project Settings.

#include "ViewModels/ScreenViewModel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ScreenViewModel)

void UScreenViewModel::Init()
{
	ensureAlways(!bHasBeenInit);
	bHasBeenInit = true;
}

void UScreenViewModel::Deinit()
{
	ensureAlways(bHasBeenInit);
	bHasBeenInit = false;

	Super::Deinit();
}
