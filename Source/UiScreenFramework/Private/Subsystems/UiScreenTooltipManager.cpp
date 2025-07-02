// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/UiScreenTooltipManager.h"

#include "Blueprint/UserWidget.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Helpers/GeneralHelper.h"
#include "Helpers/TooltipHelper.h"
#include "Logging/LogUiScreenManager.h"
#include "Subsystems/UiScreenManager.h"
#include "View/MVVMView.h"
#include "ViewModels/BaseViewModel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UiScreenTooltipManager)

void UUiScreenTooltipManager::Tick(float DeltaTime)
{
	if (!CurrentTooltip.IsNone())
	{
		const FTooltipData CurrentTooltipData = Tooltips[CurrentTooltip];
		const FVector2D NewTooltipPosition = TooltipHelper::CalculateTooltipPosition(CurrentTooltipData);
		SetTooltipPosition(CurrentTooltipData.TooltipWidget, NewTooltipPosition);

		UE_LOG(LogUiScreenFramework, Verbose, TEXT("%hs CurrentTooltipTag %s, NewTooltipPosition %s"), __FUNCTION__, *CurrentTooltip.ToString(), *NewTooltipPosition.ToString());
	}
}

void UUiScreenTooltipManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency(UUiScreenManager::StaticClass());

	SetBindings(true);
}

void UUiScreenTooltipManager::Deinitialize()
{
	SetBindings(false);

	Super::Deinitialize();
}

FTooltipData* UUiScreenTooltipManager::RegisterTooltip(const FTooltipData& TooltipCreationData)
{
	if (!ensureMsgf(TooltipCreationData.TooltipWidgetClass, TEXT("RegisterTooltip failed: TooltipWidgetClass is not set")))
	{
		return nullptr;
	}
	const FName TooltipId = FName(TooltipCreationData.TooltipWidgetClass->GetFullName());
	UUserWidget* TooltipWidget = CreateWidget<UUserWidget>(GetWorld(), TooltipCreationData.TooltipWidgetClass);
	if (!IsValid(TooltipWidget))
	{
		UE_LOG(LogUiScreenFramework, Error, TEXT("%hs RegisterTooltip failed: Could not create widget for class %s"), __FUNCTION__, *TooltipId.ToString());
		return nullptr;
	}

	SetViewModelForTooltipWidget(TooltipWidget, TooltipCreationData.TooltipViewModel);

	UOverlay* TooltipLayer = TooltipHelper::GetTooltipLayer(this);
	if (!ensure(TooltipLayer))
	{
		TooltipWidget->RemoveFromParent();
		return nullptr;
	}

	TooltipWidget->SetVisibility(ESlateVisibility::Collapsed);
	TooltipLayer->AddChild(TooltipWidget);

	FTooltipData NewTooltipData = TooltipCreationData;
	NewTooltipData.TooltipWidget = TooltipWidget;
	Tooltips.Add(TooltipId, NewTooltipData);

	return &Tooltips[TooltipId];
}

void UUiScreenTooltipManager::DisplayTooltip(const FTooltipData& TooltipRequestData)
{
	if (!ensureMsgf(TooltipRequestData.TooltipWidgetClass, TEXT("DisplayTooltip failed: TooltipWidgetClass is not set")))
	{
		return;
	}

	const FName TooltipId = FName(TooltipRequestData.TooltipWidgetClass->GetFullName());

	FTooltipData* TooltipData = Tooltips.Find(TooltipId);

	if (TooltipData)
	{
		TooltipData->WidgetGeometry = TooltipRequestData.WidgetGeometry;

		SetViewModelForTooltipWidget(TooltipData->TooltipWidget, TooltipRequestData.TooltipViewModel);
	}
	else
	{
		TooltipData = RegisterTooltip(TooltipRequestData);
	}

	if (!TooltipData)
	{
		UE_LOG(LogUiScreenFramework, Warning, TEXT("%hs DisplayTooltip failed for class %s. Could not find or register."), __FUNCTION__, *TooltipId.ToString());
		return;
	}

	CurrentTooltip = TooltipId;
	UUserWidget* CurrentTooltipWidget = Tooltips[CurrentTooltip].TooltipWidget;
	if (ensure(CurrentTooltipWidget))
	{
		CurrentTooltipWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UUiScreenTooltipManager::HideCurrentTooltip()
{
	FTooltipData* TooltipData = Tooltips.Find(CurrentTooltip);
	if (!ensure(TooltipData))
	{
		return;
	}

	UUserWidget* CurrentTooltipWidget = TooltipData->TooltipWidget;
	if (!ensure(CurrentTooltipWidget))
	{
		return;
	}

	CurrentTooltipWidget->SetVisibility(ESlateVisibility::Collapsed);
	CurrentTooltip = NAME_None;
}

void UUiScreenTooltipManager::ClearAllTooltips()
{
	for (auto& [TooltipTag, TooltipData] : Tooltips)
	{
		if (UBaseViewModel* TooltipViewModel = TooltipData.TooltipViewModel)
		{
			TooltipViewModel->Deinit();
		}

		if (UUserWidget* TooltipWidget = TooltipData.TooltipWidget)
		{
			TooltipWidget->RemoveFromParent();
			TooltipWidget = nullptr;
		}
	}

	Tooltips.Empty();
	CurrentTooltip = NAME_None;
}

void UUiScreenTooltipManager::OnUiScreenChanged(const FGameplayTag PreviousScreenTag, const FGameplayTag CurrentScreenTag)
{
	ClearAllTooltips();
}

void UUiScreenTooltipManager::SetBindings(bool bShouldBind)
{
	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!ensure(LocalPlayer))
	{
		return;
	}

	UUiScreenManager& UiScreenManager = GeneralHelper::GetLocalPlayerSubsystemChecked<UUiScreenManager>(LocalPlayer);

	if (bShouldBind)
	{
		UiScreenManager.OnUiScreenChanged.AddUObject(this, &UUiScreenTooltipManager::OnUiScreenChanged);
	}
	else
	{
		UiScreenManager.OnUiScreenChanged.RemoveAll(this);
	}
}

void UUiScreenTooltipManager::SetTooltipPosition(const TObjectPtr<UUserWidget>& TooltipWidget, FVector2D TooltipPosition) const
{
	const UOverlay* TooltipLayer = TooltipHelper::GetTooltipLayer(this);
	if (!ensure(TooltipLayer))
	{
		return;
	}

	UOverlaySlot* Slot = Cast<UOverlaySlot>(TooltipWidget->Slot);

	if (ensure(Slot))
	{
		Slot->SetPadding(FMargin(TooltipPosition.X, TooltipPosition.Y, 0, 0));
	}
}

void UUiScreenTooltipManager::SetViewModelForTooltipWidget(const UUserWidget* TooltipWidget, UBaseViewModel* TooltipViewModel)
{
	if (!IsValid(TooltipViewModel))
	{
		return;
	}

	if (!ensure(TooltipWidget))
	{
		return;
	}

	UMVVMView* View = TooltipWidget->GetExtension<UMVVMView>();
	if (!IsValid(View))
	{
		UE_LOG(LogUiScreenFramework, Warning, TEXT("%hs Widget %s does not have a view model %s"), __FUNCTION__, *GetNameSafe(TooltipWidget), *GetNameSafe(TooltipViewModel));
		return;
	}

	View->SetViewModelByClass(TooltipViewModel);
}
