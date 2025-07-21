// Copyright People Can Fly. All Rights Reserved."

#pragma once


class UMVVMView;
class UBaseViewModel;
class UUserWidget;

namespace ViewModelHelper
{
	UMVVMView* GetMvvmViewExtensionFromWidget(const UUserWidget* Widget);
	bool SetViewModel(const UUserWidget* Widget, UBaseViewModel* ViewModel);
}
