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
UE_DISABLE_OPTIMIZATION
void UUiScreenTooltipManager::Tick(float DeltaTime)
{
	if (CurrentTooltipTag.IsValid())
	{
		const FTooltipData CurrentTooltipData = Tooltips[CurrentTooltipTag];
		const FVector2D NewTooltipPosition = TooltipHelper::CalculateTooltipPosition(CurrentTooltipData);
		SetTooltipPosition(CurrentTooltipData.TooltipWidget, NewTooltipPosition);
		UE_LOG(LogUiScreenFramework, Verbose, TEXT("%hs CurrentTooltipTag %s, NewTooltipPosition %s"), __FUNCTION__, *CurrentTooltipTag.ToString(), *NewTooltipPosition.ToString());
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
	if (!ensureMsgf(TooltipCreationData.TooltipWidgetClass, TEXT("RegisterTooltip failed: TooltipWidgetClass is null for tag %s"), *TooltipCreationData.TooltipTag.ToString()))
	{
		return nullptr;
	}

	UUserWidget* TooltipWidget = CreateWidget<UUserWidget>(GetWorld(), TooltipCreationData.TooltipWidgetClass);
	if (!IsValid(TooltipWidget))
	{
		UE_LOG(LogUiScreenFramework, Error, TEXT("%hs RegisterTooltip failed: Could not create widget for tag %s"), __FUNCTION__, *TooltipCreationData.TooltipTag.ToString());
		return nullptr;
	}

	if (IsValid(TooltipCreationData.TooltipViewModel))
	{
		if (UMVVMView* View = TooltipWidget->GetExtension<UMVVMView>())
		{
			View->SetViewModelByClass(TooltipCreationData.TooltipViewModel);
		}
		else
		{
			UE_LOG(LogUiScreenFramework, Warning, TEXT("%hs RegisterTooltip: Widget for tag %s does not have a UMVVMView extension."), __FUNCTION__, *TooltipCreationData.TooltipTag.ToString());
		}
	}

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
	Tooltips.Add(NewTooltipData.TooltipTag, NewTooltipData);

	return &Tooltips[NewTooltipData.TooltipTag];
}

void UUiScreenTooltipManager::DisplayTooltip(const FTooltipData& TooltipRequestData)
{
	const FGameplayTag& TooltipTag = TooltipRequestData.TooltipTag;

	const FTooltipData* TooltipData = Tooltips.Find(TooltipTag);

	if (!TooltipData)
	{
		TooltipData = RegisterTooltip(TooltipRequestData);
	}

	if (!TooltipData)
	{
		UE_LOG(LogUiScreenFramework, Warning, TEXT("%hs DisplayTooltip failed for tag %s. Could not find or register."), __FUNCTION__, *TooltipTag.ToString());
		return;
	}

	CurrentTooltipTag = TooltipData->TooltipTag;
	UUserWidget* CurrentTooltipWidget = Tooltips[CurrentTooltipTag].TooltipWidget;
	if (ensure(CurrentTooltipWidget))
	{
		CurrentTooltipWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UUiScreenTooltipManager::HideCurrentTooltip()
{
	FTooltipData* TooltipData = Tooltips.Find(CurrentTooltipTag);
	if (!ensure(TooltipData))
	{
		return;
	}

	UUserWidget* CurrentTooltipWidget = TooltipData->TooltipWidget;
	if (!ensure(CurrentTooltipWidget))
	{
		return;
	}

	CurrentTooltipWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	CurrentTooltipTag = FGameplayTag::EmptyTag;
}

void UUiScreenTooltipManager::ClearAllTooltips()
{
	for (auto& [TooltipTag, TooltipData] : Tooltips)
	{
		if (UBaseViewModel* TooltipViewModel = TooltipData.TooltipViewModel)
		{
			TooltipViewModel->Deinit();
		}

		TooltipData.TooltipWidget = nullptr;
	}

	CurrentTooltipTag = FGameplayTag::EmptyTag;
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
UE_DISABLE_OPTIMIZATION