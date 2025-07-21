// Copyright People Can Fly. All Rights Reserved."


#include "Helpers/ViewModelHelper.h"

#include "Blueprint/UserWidget.h"
#include "Logging/LogUiScreenManager.h"
#include "ViewModels/BaseViewModel.h"
#include "View/MVVMView.h"

UMVVMView* ViewModelHelper::GetMvvmViewExtensionFromWidget(const UUserWidget* Widget)
{
	if (!IsValid(Widget))
	{
		return nullptr;
	}
	
	return Widget->GetExtension<UMVVMView>();
}

bool ViewModelHelper::SetViewModel(const UUserWidget* Widget, UBaseViewModel* ViewModel)
{
	UMVVMView* ViewExtension = GetMvvmViewExtensionFromWidget(Widget);
	if (!ViewExtension)
	{
		UE_LOG(LogUiScreenFramework, Error,
			TEXT("%hs : MVVM view extension could not be found from widget %s, function abandoned"), __FUNCTION__, *GetNameSafe(Widget));
		return false;
	}

	return ViewExtension->SetViewModelByClass(ViewModel);
}
