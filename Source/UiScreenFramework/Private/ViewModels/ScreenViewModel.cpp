// Fill out your copyright notice in the Description page of Project Settings.


#include "ViewModels/ScreenViewModel.h"

void UScreenViewModel::Init()
{
	ensureAlways(!bHasBeenInit);
	bHasBeenInit = true;
}

void UScreenViewModel::Deinit()
{
	ensureAlways(bHasBeenInit);
	bHasBeenInit = false;

	SetBindings(false);
	CleanAllPointers();
}
