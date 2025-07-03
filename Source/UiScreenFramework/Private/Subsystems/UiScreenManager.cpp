// Copyright People Can Fly. All Rights Reserved."

#include "Subsystems/UiScreenManager.h"

#include "CommonActivatableWidget.h"
#include "DeveloperSettings/UiScreenFrameworkSettings.h"
#include "Engine/AssetManager.h"
#include "Helpers/UiScreenManagerHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/LogUiScreenManager.h"
#include "ViewModels/ScreenViewModel.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(UiScreenManager)

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
		UE_LOG(LogUiScreenFramework, Error, TEXT("%hs : PlayerController is invalid cannot get UiScreenManager"), __FUNCTION__);
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

void UUiScreenManager::CleanupAllScreenViewModels()
{
	for (auto& [ScreenTag, ScreenViewModel] : ScreenViewModelsMap)
	{
		DeinitScreenViewModel(ScreenViewModel);
	}

	ScreenViewModelsMap.Empty();
}

void UUiScreenManager::Deinitialize()
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	check(IsValid(LocalPlayer));

	LocalPlayer->OnPlayerControllerChanged().RemoveAll(this);

	CleanupAllScreenViewModels();

	ScreensToDisplayQueue.Empty();
	InitializeScreenWidgetCallback.Reset();

	if (UMainUiLayoutWidget* MainLayoutWidget = MainLayoutWidgetInfo.MainLayoutWidget)
	{
		MainLayoutWidget->OnDisplayedWidgetChangedDelegate.Unbind();
	}

	if (StreamingHandle.IsValid())
	{
		StreamingHandle.Reset();
	}

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

void UUiScreenManager::OnScreenWidgetClassLoaded(FScreenInitialData ScreenInitialData)
{
	UE_LOG(LogUiScreenFramework, Log, TEXT("%hs Screen %s"), __FUNCTION__, *ScreenInitialData.ScreenTag.ToString());

	FUiScreenInfo* FoundViewInfo = GetUiScreenInfo(ScreenInitialData.ScreenTag);
	if (!FoundViewInfo)
	{
		return;
	}

	UMainUiLayoutWidget* MainLayoutWidget = MainLayoutWidgetInfo.MainLayoutWidget;
	if (!ensure(MainLayoutWidget))
	{
		return;
	}

	CreateOrReuseScreenViewModel(ScreenInitialData, *FoundViewInfo);
	SetCurrentScreenState(ScreenInitialData, *FoundViewInfo);
	InitializeScreenWidgetCallback = ScreenInitialData.InitializeScreenWidgetCallback;

	MainLayoutWidget->SetWidgetForLayer(*FoundViewInfo, ScreenInitialData.bCleanUpExistingScreens);

	const FGameplayTag PreviousScreenTag = CurrentScreenState.ScreenId;
	const FGameplayTag NewScreenTag = FoundViewInfo->ScreenId;
	BroadcastScreenChange(PreviousScreenTag, NewScreenTag);

	// check whether other screens waiting to be displayed
	FScreenInitialData EnqueuedScreenData;
	ScreensToDisplayQueue.Dequeue(EnqueuedScreenData);
	if (EnqueuedScreenData.ScreenTag.IsValid())
	{
		ChangeUiScreen(EnqueuedScreenData);
	}
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
			MainUiLayoutWidget->OnDisplayedWidgetChangedDelegate.BindUObject(this, &UUiScreenManager::CallInitializeScreenWidgetCallback);
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

void UUiScreenManager::DeinitScreenViewModel(UScreenViewModel* ScreenViewModel)
{
	if (IsValid(ScreenViewModel))
	{
		ScreenViewModel->Deinit();
		ScreenViewModel = nullptr;
	}
}

void UUiScreenManager::SetCurrentScreenState(const FScreenInitialData& ScreenInitialData, const FUiScreenInfo& FoundViewInfo)
{
	const FGameplayTag& ScreenId = FoundViewInfo.ScreenId;
	if (CurrentScreenState.ScreenId.IsValid())
	{
		CurrentScreenState.PreviousScreens.Add(CurrentScreenState.ScreenId);
	}

	if (ScreenInitialData.bCleanUpExistingScreens)
	{
		CurrentScreenState.PreviousScreens.Empty();
	}

	int32 ExistingScreenIndex = -1;
	if (CurrentScreenState.PreviousScreens.Find(ScreenId, ExistingScreenIndex))
	{
		const int32 NumElementsToRemove = CurrentScreenState.PreviousScreens.Num() - ExistingScreenIndex;

		for (int32 Index = ExistingScreenIndex + 1; Index < CurrentScreenState.PreviousScreens.Num(); ++Index)
		{
			FGameplayTag RemovedTag = CurrentScreenState.PreviousScreens[Index];
			UScreenViewModel* ScreenViewModel = GetScreenViewModel(RemovedTag);
			DeinitScreenViewModel(ScreenViewModel);
			ScreenViewModelsMap.Remove(RemovedTag);
		}

		CurrentScreenState.PreviousScreens.RemoveAt(ExistingScreenIndex, NumElementsToRemove);
	}

	CurrentScreenState.ScreenId = ScreenId;
	CurrentScreenState.LayerId = FoundViewInfo.LayerId;
}

void UUiScreenManager::CreateOrReuseScreenViewModel(const FScreenInitialData& ScreenInitialData, const FUiScreenInfo& FoundViewInfo)
{
	const TSoftClassPtr<UScreenViewModel> ScreeViewModelClass = FoundViewInfo.ScreenViewModelClass;
	if (ScreeViewModelClass.IsNull())
	{
		UE_LOG(LogUiScreenFramework, Warning, TEXT("%hs ScreeViewModelClass is empty for %s Screen View model won't be created"), __FUNCTION__, *FoundViewInfo.ScreenId.ToString());
		return;
	}

	if (ScreenInitialData.bCleanUpExistingScreens)
	{
		CleanupAllScreenViewModels();
	}
	
	UScreenViewModel* ScreenViewModel = GetScreenViewModel(FoundViewInfo.ScreenId);

	if (!IsValid(ScreenViewModel))
	{
		const UClass* ScreenViewModelClass = ScreeViewModelClass.LoadSynchronous();
		ScreenViewModel = NewObject<UScreenViewModel>(this, ScreenViewModelClass);
		ScreenViewModel->Init();

		ScreenViewModelsMap.Add(FoundViewInfo.ScreenId, ScreenViewModel);
	}

	if (ScreenInitialData.InitializeViewModelCallback)
	{
		ScreenInitialData.InitializeViewModelCallback(ScreenViewModel);
	}
}

void UUiScreenManager::BroadcastScreenChange(const FGameplayTag PreviousScreenTag, const FGameplayTag CurrentScreenTag) const
{
	OnUiScreenChanged.Broadcast(PreviousScreenTag, CurrentScreenTag);
	OnUiScreenChanged_BP.Broadcast(PreviousScreenTag, CurrentScreenTag);
}

void UUiScreenManager::ChangeUiScreen(FScreenInitialData ScreenInitialData)
{
	const FGameplayTag ScreenTag = ScreenInitialData.ScreenTag;

	const FUiScreenInfo* FoundViewInfo = GetUiScreenInfo(ScreenTag);
	if (!FoundViewInfo)
	{
		return;
	}

	if (ScreenTag == CurrentScreenState.ScreenId)
	{
		UE_LOG(LogUiScreenFramework, Error, TEXT("%hs New screen id %s is the same as current one, change aborted"), __FUNCTION__, *ScreenTag.ToString());
		return;
	}

	TSoftClassPtr<UCommonActivatableWidget> ScreenWidgetSoftClass = FoundViewInfo->ScreenClass;
	if (ScreenWidgetSoftClass.IsNull())
	{
		UE_LOG(LogUiScreenFramework, Error, TEXT("%hs Screen class isn't set up for screen %s"), __FUNCTION__, *ScreenTag.ToString());
		return;
	}

	if (StreamingHandle && StreamingHandle->IsLoadingInProgress())
	{
		UE_LOG(LogUiScreenFramework, Log, TEXT("%hs Enqueue %s screen"), __FUNCTION__, *ScreenTag.ToString());
		ScreensToDisplayQueue.Enqueue(ScreenInitialData);
		return;
	}

	if (ScreenWidgetSoftClass.IsValid())
	{
		OnScreenWidgetClassLoaded(ScreenInitialData);
	}
	else
	{
		StreamingHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
			ScreenWidgetSoftClass.ToSoftObjectPath(),
			FStreamableDelegate::CreateUObject(this, &ThisClass::OnScreenWidgetClassLoaded, ScreenInitialData)
			);
	}
}

void UUiScreenManager::GoToThePreviousUiScreen()
{
	if (CurrentScreenState.PreviousScreens.IsEmpty())
	{
		UE_LOG(LogUiScreenFramework, Error, TEXT("%hs Cannot go back to the previous screen, no previous screen cached"), __FUNCTION__);
		return;
	}

	ChangeUiScreen(FScreenInitialData(CurrentScreenState.PreviousScreens.Last()));
}

void UUiScreenManager::AddMainLayoutToViewport(UMainUiLayoutWidget* LayoutWidget)
{
	UE_LOG(LogUiScreenFramework, Log, TEXT("%hs %s is adding the root layout %s to the viewport"), __FUNCTION__, *GetName(), *GetNameSafe(LayoutWidget));

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
		UE_LOG(LogUiScreenFramework, Log, TEXT("%s is removing root layout %s from the viewport"), *GetName(), *LayoutWidget.GetName());

		LayoutWidget.RemoveFromParent();
		if (LayoutSlateWidget.IsValid())
		{
			UE_LOG(LogUiScreenFramework, Log, TEXT("Root layout [%s] has been removed from the viewport, but other references to its"
				" underlying Slate widget still exist. Noting in case we leak it."), *LayoutWidget.GetName());
		}
	}
}

FUiScreenInfo* UUiScreenManager::GetUiScreenInfo(const FGameplayTag ScreenTag)
{
	const UUiScreenFrameworkSettings& ScreenFrameworkSettings = UiScreenManagerHelper::GetUiScreenFrameworkSettings();
	UUiScreensData* UiScreensData = ScreenFrameworkSettings.GetViewsData();
	FUiScreenInfo* FoundViewInfo = UiScreensData->Screens.FindByPredicate([ScreenTag](const FUiScreenInfo& ViewInfo)
	{
		return ViewInfo.ScreenId == ScreenTag;
	});

	if (!FoundViewInfo)
	{
		UE_LOG(LogUiScreenFramework, Error, TEXT("%hs ViewInfo with ScreenTag %s hasn't been found in View Data asset"), __FUNCTION__, *ScreenTag.ToString());
		return nullptr;
	}

	return FoundViewInfo;
}

UScreenViewModel* UUiScreenManager::GetScreenViewModel(const FGameplayTag ScreenTag)
{
	TObjectPtr<UScreenViewModel>* ScreenViewModelPtr = ScreenViewModelsMap.Find(ScreenTag);
	if (!ScreenViewModelPtr)
	{
		return nullptr;
	}

	return *ScreenViewModelPtr;
}

UCommonActivatableWidget* UUiScreenManager::GetScreenWidget() const
{
	UMainUiLayoutWidget* MainLayoutWidget = MainLayoutWidgetInfo.MainLayoutWidget;
	if (!ensureAlways(MainLayoutWidget))
	{
		return nullptr;
	}

	return MainLayoutWidget->GetCurrentScreenWidget();
}

void UUiScreenManager::CallInitializeScreenWidgetCallback(UCommonActivatableWidget* ScreenWidget)
{
	if (InitializeScreenWidgetCallback)
	{
		InitializeScreenWidgetCallback(ScreenWidget);
	}
}
