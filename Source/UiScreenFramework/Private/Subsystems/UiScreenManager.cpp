// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/UiScreenManager.h"

#include "DeveloperSettings/UiScreenFrameworkSettings.h"
#include "Helpers/UiScreenManagerHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/LogUiScreenManager.h"
#include "ViewModels/ScreenViewModel.h"

UE_DISABLE_OPTIMIZATION
UUiScreenManager* UUiScreenManager::Get(const AController* Controller)
{
	if (const APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			return LocalPlayer->GetSubsystem<UUiScreenManager>();
		}
	}

	return nullptr;
}

UUiScreenManager* UUiScreenManager::Get(const UObject* WorldContextObject)
{
	if (!IsValid(WorldContextObject))
	{
		UE_LOG(LogUiScreenFramework, Error, TEXT("%hs : WorldContextObject is invalid cannot get UiScreenManager"), __FUNCTION__);
		return nullptr;
	}

	const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	if (!IsValid(PlayerController))
	{
		UE_LOG(LogUiScreenFramework, Error, TEXT("%hs : PlayerController is invalid cannot get UiScreenManager"),__FUNCTION__);
		return nullptr;
	}

	return UUiScreenManager::Get(PlayerController);
}

UUiScreenManager& UUiScreenManager::GetChecked(const UObject* WorldContextObject)
{
	UUiScreenManager* UiScreenManager = Get(WorldContextObject);
	checkf(IsValid(UiScreenManager), TEXT("UiScreenManager was invalid"));
	return *UiScreenManager;
}

void UUiScreenManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	check(IsValid(LocalPlayer));

	LocalPlayer->OnPlayerControllerChanged().AddUObject(this, &UUiScreenManager::InitializeUiScreenManager);

	APlayerController* PlayerController = LocalPlayer->GetPlayerController(GetWorld());
	if (PlayerController)
	{
		InitializeUiScreenManager(PlayerController);
	}
}

void UUiScreenManager::Deinitialize()
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	check(IsValid(LocalPlayer));

	LocalPlayer->OnPlayerControllerChanged().RemoveAll(this);

	DeinitCurrentScreenViewModel();

	Super::Deinitialize();
}

void UUiScreenManager::InitializeUiScreenManager(APlayerController* PlayerController)
{
	if (MainLayoutWidgetInfo.bAddedToViewport)
	{
		RemoveMainLayoutWidget();
	}

	CreateMainLayoutWidget(PlayerController);
}

void UUiScreenManager::CreateMainLayoutWidget(APlayerController* PlayerController)
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	check(IsValid(LocalPlayer));

	ensure(LocalPlayer->GetPlayerController(GetWorld()) == PlayerController);

	UE_LOG(LogUiScreenFramework, Log, TEXT("%hs : RefreshLayout"), __FUNCTION__);

	if (ensure(PlayerController))
	{
		const UUiScreenFrameworkSettings& ScreenFrameworkSettings = UiScreenManagerHelper::GetUiScreenFrameworkSettings();
		const TSubclassOf<UMainUiLayoutWidget> LayoutWidgetClassLoaded = ScreenFrameworkSettings.GetLayoutWidgetClass();

		if (ensure(LayoutWidgetClassLoaded && !LayoutWidgetClassLoaded->HasAnyClassFlags(CLASS_Abstract)))
		{
			UMainUiLayoutWidget* MainUiLayoutWidget = CreateWidget<UMainUiLayoutWidget>(PlayerController, LayoutWidgetClassLoaded);
			MainLayoutWidgetInfo = {LocalPlayer, MainUiLayoutWidget, true};

			AddMainLayoutToViewport(MainUiLayoutWidget);
		}
	}

	UE_LOG(LogUiScreenFramework, Log, TEXT("%hs : UIViewManager Layout initialized"), __FUNCTION__);
}

void UUiScreenManager::RemoveMainLayoutWidget()
{
	if (!IsValid(MainLayoutWidgetInfo.MainLayoutWidget))
	{
		MainLayoutWidgetInfo.MainLayoutWidget = nullptr;
		return;
	}

	if (!MainLayoutWidgetInfo.bAddedToViewport)
	{
		return;
	}

	RemoveMainLayoutFromViewport(*MainLayoutWidgetInfo.MainLayoutWidget);
	MainLayoutWidgetInfo.bAddedToViewport = false;
}

void UUiScreenManager::DeinitCurrentScreenViewModel()
{
	if (IsValid(CurrentScreenState.ScreeViewModel))
	{
		CurrentScreenState.ScreeViewModel->Deinit();
		CurrentScreenState.ScreeViewModel = nullptr;
	}
}

void UUiScreenManager::SetCurrentScreenState(const FUiScreenInfo& FoundViewInfo)
{
	const FGameplayTag& ScreenId = FoundViewInfo.ScreenId;

	int32 ExistingScreenIndex;
	if (CurrentScreenState.PreviousScreens.Find(ScreenId, ExistingScreenIndex))
	{
		const int32 NumElementsToRemove = CurrentScreenState.PreviousScreens.Num() - ExistingScreenIndex;

		CurrentScreenState.PreviousScreens.RemoveAt(ExistingScreenIndex, NumElementsToRemove);
	}
	else
	{
		if (CurrentScreenState.ScreenId.IsValid())
		{
			CurrentScreenState.PreviousScreens.Add(CurrentScreenState.ScreenId);
		}
	}

	CurrentScreenState.ScreenId = ScreenId;
	CurrentScreenState.LayerId = FoundViewInfo.LayerId;
}

void UUiScreenManager::CreateAndInitializeScreenViewModel(TFunction<void(UScreenViewModel*)> InitializeViewModelCallback, const FUiScreenInfo& FoundViewInfo)
{
	TSoftClassPtr<UScreenViewModel> ScreeViewModelClass = FoundViewInfo.ScreeViewModelClass;

	if (!ScreeViewModelClass.IsNull())
	{
		UScreenViewModel* ScreenViewModel = nullptr;
		const UClass* ScreenViewModelClass = ScreeViewModelClass.LoadSynchronous();
		ScreenViewModel = NewObject<UScreenViewModel>(this, ScreenViewModelClass);
		CurrentScreenState.ScreeViewModel = ScreenViewModel;

		ScreenViewModel->Init();

		if (InitializeViewModelCallback)
		{
			InitializeViewModelCallback(ScreenViewModel);
		}
	}
	else
	{
		UE_LOG(LogUiScreenFramework, Warning, TEXT("%hs ScreeViewModelClass is empty for %s Screen View model won't be created"), __FUNCTION__, *FoundViewInfo.ScreenId.ToString());
	}
}

void UUiScreenManager::BroadcastScreenChange(const FGameplayTag PreviousScreenTag, const FGameplayTag CurrentScreenTag) const
{
	OnUiScreenChanged.Broadcast(PreviousScreenTag, CurrentScreenTag);
	OnUiScreenChanged_BP.Broadcast(PreviousScreenTag, CurrentScreenTag);
}

void UUiScreenManager::ChangeUiScreen(const FGameplayTag ScreenTag, TFunction<void(UScreenViewModel*)> InitializeViewModelCallback)
{
	const UUiScreenFrameworkSettings& ScreenFrameworkSettings = UiScreenManagerHelper::GetUiScreenFrameworkSettings();
	UUiScreensData* UiScreensData = ScreenFrameworkSettings.GetViewsData();
	const FUiScreenInfo* FoundViewInfo = UiScreensData->Screens.FindByPredicate([ScreenTag](const FUiScreenInfo& ViewInfo)
	{
		return ViewInfo.ScreenId == ScreenTag;
	});

	if (!FoundViewInfo)
	{
		UE_LOG(LogUiScreenFramework, Error, TEXT("%hs ViewInfo with ScreenTag %s hasn't been found in View Data asset"), __FUNCTION__, *ScreenTag.ToString());
		return;
	}

	UMainUiLayoutWidget* MainLayoutWidget = MainLayoutWidgetInfo.MainLayoutWidget;
	if (!ensure(MainLayoutWidget))
	{
		return;
	}

	if (ScreenTag == CurrentScreenState.ScreenId)
	{
		UE_LOG(LogUiScreenFramework, Error, TEXT("%hs New screen id %s is the same as current one, change aborted"), __FUNCTION__, *ScreenTag.ToString());
		return;
	}

	DeinitCurrentScreenViewModel();

	CreateAndInitializeScreenViewModel(InitializeViewModelCallback, *FoundViewInfo);
	const FGameplayTag PreviousScreenTag = CurrentScreenState.ScreenId;
	const FGameplayTag CurrentScreenTag = FoundViewInfo->ScreenId;

	SetCurrentScreenState(*FoundViewInfo);

	MainLayoutWidget->SetWidgetForLayer(*FoundViewInfo);

	BroadcastScreenChange(PreviousScreenTag, CurrentScreenTag);
}

void UUiScreenManager::GoToThePreviousUiScreen()
{
	if (CurrentScreenState.PreviousScreens.IsEmpty())
	{
		UE_LOG(LogUiScreenFramework, Error, TEXT("%hs Cannot go back to the previous screen, no previous screen cached"), __FUNCTION__);
		return;
	}

	ChangeUiScreen(CurrentScreenState.PreviousScreens.Last());
}

void UUiScreenManager::AddMainLayoutToViewport(UMainUiLayoutWidget* LayoutWidget)
{
	UE_LOG(LogUiScreenFramework, Log, TEXT("%hs [%s] is adding the root layout [%s] to the viewport"), __FUNCTION__, *GetName(), *GetNameSafe(LayoutWidget));

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	check(IsValid(LocalPlayer));

	const FLocalPlayerContext LocalPlayerContext = FLocalPlayerContext(LocalPlayer);
	LayoutWidget->SetPlayerContext(LocalPlayerContext);
	LayoutWidget->AddToPlayerScreen(MainLayoutWidgetZOrder);
}

void UUiScreenManager::RemoveMainLayoutFromViewport(UMainUiLayoutWidget& LayoutWidget)
{
	const TWeakPtr<SWidget> LayoutSlateWidget = LayoutWidget.GetCachedWidget();
	if (LayoutSlateWidget.IsValid())
	{
		UE_LOG(LogUiScreenFramework, Log, TEXT("[%s] is removing root layout [%s] from the viewport"), *GetName(), *LayoutWidget.GetName());

		LayoutWidget.RemoveFromParent();
		if (LayoutSlateWidget.IsValid())
		{
			UE_LOG(LogUiScreenFramework, Log, TEXT("Root layout [%s] has been removed from the viewport, but other references to its"
				" underlying Slate widget still exist. Noting in case we leak it."), *LayoutWidget.GetName());
		}
	}
}
UE_ENABLE_OPTIMIZATION