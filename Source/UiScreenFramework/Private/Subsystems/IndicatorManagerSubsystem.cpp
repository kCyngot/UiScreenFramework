// Copyright People Can Fly. All Rights Reserved.

#include "Subsystems/IndicatorManagerSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "ViewModels/BaseIndicatorViewModel.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(IndicatorManagerSubsystem)

DEFINE_LOG_CATEGORY(LogUIIndicatorPanel);

void UIndicatorManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	constexpr float VisibilityCheckRefreshRate = 0.25f;
	constexpr bool bRefreshInLoop = true;
	GetWorld()->GetTimerManager().SetTimer(DistanceCheckTimerHandle, this, &UIndicatorManagerSubsystem::HandleDistanceCheck, VisibilityCheckRefreshRate, bRefreshInLoop);
	GetWorld()->GetTimerManager().SetTimer(VisibilityCheckTimerHandle, this, &UIndicatorManagerSubsystem::HandleVisibilityCheck, VisibilityCheckRefreshRate, bRefreshInLoop);
	OnWorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddUObject(this, &UIndicatorManagerSubsystem::OnWorldCleanup);
}

void UIndicatorManagerSubsystem::Deinitialize()
{
	FWorldDelegates::OnWorldCleanup.Remove(OnWorldCleanupHandle);
	if (VisibilityCheckTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(VisibilityCheckTimerHandle);
	}
	if (DistanceCheckTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(DistanceCheckTimerHandle);
	}

	OnIndicatorAdded.Clear();
	OnIndicatorRemoved.Clear();
	OnIndicatorCategoryVisibilityChanged.Clear();
	Indicators.Empty();
	UE_LOG(LogUIIndicatorPanel, Log, TEXT("UIndicatorManagerSubsystem::Deinitialize"));
	Super::Deinitialize();
}

UIndicatorManagerSubsystem* UIndicatorManagerSubsystem::Get(const APlayerController* PlayerController)
{
	if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
	{
		return LocalPlayer->GetSubsystem<UIndicatorManagerSubsystem>();
	}

	return nullptr;
}

void UIndicatorManagerSubsystem::AddIndicator(UBaseIndicatorViewModel* IndicatorViewModel)
{
#if WITH_SERVER_CODE
	if (IsRunningDedicatedServer())
	{
		UE_LOG(LogUIIndicatorPanel, Error, TEXT("Initializing UI component on the server is forbidden!"));
		return;
	}
#endif

	if (!IndicatorViewModel)
	{
		UE_LOG(LogUIIndicatorPanel, Warning, TEXT("%s : IndicatorViewModel is not valid"), *FString(__FUNCTION__));
		return;
	}

	UE_LOG(LogUIIndicatorPanel, Verbose, TEXT("%s : IndicatorViewModel %s, AttachedToActor %s"), *FString(__FUNCTION__), *GetNameSafe(IndicatorViewModel),
		*GetNameSafe(IndicatorViewModel->GetActorAttachedTo()));
	Indicators.Add(IndicatorViewModel);
	OnIndicatorAdded.Broadcast(IndicatorViewModel);
}

void UIndicatorManagerSubsystem::RemoveIndicator(UBaseIndicatorViewModel* IndicatorViewModel)
{
#if !UE_SERVER
	if (!IsRunningDedicatedServer())
	{
		if (!IndicatorViewModel)
		{
			UE_LOG(LogUIIndicatorPanel, Warning, TEXT("%s : IndicatorViewModel is not valid"), *FString(__FUNCTION__));
			return;
		}

		UE_LOG(LogUIIndicatorPanel, Verbose, TEXT("%s : IndicatorViewModel %s, AttachedToActor %s"), *FString(__FUNCTION__), *GetNameSafe(IndicatorViewModel),
			*GetNameSafe(IndicatorViewModel->GetActorAttachedTo()));

		IndicatorViewModel->Deinit();

		Indicators.Remove(IndicatorViewModel);
		OnIndicatorRemoved.Broadcast(IndicatorViewModel);
	}
#endif
}

void UIndicatorManagerSubsystem::SetIndicatorVisibilityOption(const int32 NewVisibilityOption)
{
	IndicatorVisibilityOption = NewVisibilityOption;
	OnIndicatorCategoryVisibilityChanged.Broadcast(NewVisibilityOption);
}

#if !UE_BUILD_SHIPPING
FString UIndicatorManagerSubsystem::GetIndicatorDebugInfo(const UBaseIndicatorViewModel* IndicatorViewModel, const bool bShowExtended)
{
	FString IndicatorViewModelInfo;
	if (IndicatorViewModel)
	{
		if (bShowExtended)
		{
			IndicatorViewModelInfo.Append(
				FString::Format(
					TEXT(
						"IndicatorWidget: {0} \nVisibility: {1} \nActorAttachedTo: {2}"
						"\nIndicatorWidgetClass: {3} \nProjectionMode: {4}, HAlignment: {5}, VAlignment: {6}"
						"\nBoundingBoxAnchor: {7}, ScreenSpaceOffset: {8}, WorldPositionOffset: {9}"
						"\nbClampToScreen: {10}, Priority: {11}"
						),
					{
						IndicatorViewModel->IndicatorWidget.IsValid() ? IndicatorViewModel->IndicatorWidget->GetName() : TEXT("None"),
						IndicatorViewModel->GetIndicatorVisibility() ? TEXT("True") : TEXT("False"),
						IndicatorViewModel->GetActorAttachedTo() ? IndicatorViewModel->GetActorAttachedTo()->GetName() : TEXT("None"),
						IndicatorViewModel->GetIndicatorClass().ToString(),
						UEnum::GetValueAsString(IndicatorViewModel->GetProjectionMode()),
						UEnum::GetValueAsString(IndicatorViewModel->GetHAlign()),
						UEnum::GetValueAsString(IndicatorViewModel->GetVAlign()),
						IndicatorViewModel->GetBoundingBoxAnchor().ToString(),
						IndicatorViewModel->GetScreenSpaceOffset().ToString(),
						IndicatorViewModel->GetWorldPositionOffset().ToString(),
						IndicatorViewModel->GetClampToScreen() ? TEXT("True") : TEXT("False"),
						FString::FromInt(IndicatorViewModel->GetPriority())
					}
					)
				);
		}
		else
		{
			IndicatorViewModelInfo.Append(
				FString::Format(
					TEXT("IndicatorWidget: {0} \nVisibility: {1} \nActorAttachedTo: {2}"),
					{
						IndicatorViewModel->IndicatorWidget.IsValid() ? IndicatorViewModel->IndicatorWidget->GetName() : TEXT("None"),
						IndicatorViewModel->GetIndicatorVisibility() ? TEXT("True") : TEXT("False"),
						IndicatorViewModel->GetActorAttachedTo() ? IndicatorViewModel->GetActorAttachedTo()->GetName() : TEXT("None")
					}
					)
				);
		}
	}
	else
	{
		IndicatorViewModelInfo.Append(TEXT("IndicatorViewModel is null"));
	}
	return IndicatorViewModelInfo;
}

void UIndicatorManagerSubsystem::DumpIndicatorPanelInfo(const bool bShowOnScreen, const bool bShowExtended, const float DisplayTime) const
{
	const FLinearColor Color = FColor::White;
	const FVector2D FontSize = FVector2D::One();
	const FString IndicatorViewModelsInfo = GetIndicatorPanelInfo(bShowExtended);

	if (bShowOnScreen && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, DisplayTime, Color.ToFColor(true), IndicatorViewModelsInfo, true, FontSize);
	}

	UE_LOG(LogUIIndicatorPanel, Log, TEXT("%s"), *IndicatorViewModelsInfo);
}

FString UIndicatorManagerSubsystem::GetIndicatorPanelInfo(const bool bShowExtended) const
{
	const FString StartingMessage = FString::Format(TEXT("IndicatorViewModel array dump. Number of indicators - {0}"), {GetIndicators().Num()});

	FString IndicatorViewModelsInfo;
	IndicatorViewModelsInfo.Append(StartingMessage);
	for (const UBaseIndicatorViewModel* IndicatorViewModel : GetIndicators())
	{
		IndicatorViewModelsInfo.Append(TEXT("\n========================\n"));
		const FString IndicatorViewModelInfo = GetIndicatorDebugInfo(IndicatorViewModel, bShowExtended);
		IndicatorViewModelsInfo.Append(IndicatorViewModelInfo);
	}

	IndicatorViewModelsInfo.Append(TEXT("\n========================"));

	return IndicatorViewModelsInfo;
}
#endif // !UE_BUILD_SHIPPING

void UIndicatorManagerSubsystem::HandleVisibilityCheck()
{
	for (UBaseIndicatorViewModel* IndicatorViewModel : Indicators)
	{
		if (IndicatorViewModel && IndicatorViewModel->GetHasVisibilityRange())
		{
			IndicatorViewModel->HandleVisibilityCheck();
		}
	}
}

void UIndicatorManagerSubsystem::HandleDistanceCheck()
{
	for (UBaseIndicatorViewModel* IndicatorViewModel : Indicators)
	{
		if (IndicatorViewModel && IndicatorViewModel->ShouldUpdateDistance())
		{
			IndicatorViewModel->UpdateDistanceFactor();
		}
	}
}

void UIndicatorManagerSubsystem::OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	for (auto It = Indicators.CreateIterator(); It; It++)
	{
		if (UBaseIndicatorViewModel* IndicatorViewModel = *It)
		{
			if (AActor* IndicatorActor = IndicatorViewModel->GetActorAttachedTo())
			{
				if (IndicatorActor->GetWorld() == World)
				{
#if !UE_BUILD_SHIPPING
					UE_LOG(LogUIIndicatorPanel, Warning,
						TEXT("In UIndicatorManagerSubsystem::OnWorldCleanup - indicator attached to an actor in world %s ")
						TEXT("was not removed via UIndicatorManagerSubsystem::RemoveIndicator; will drop it now to avoid leaking the world. %s"),
						*GetNameSafe(World), *GetIndicatorDebugInfo(IndicatorViewModel));
#endif
					It.RemoveCurrentSwap();
				}
			}
		}
	}
}
