// Copyright People Can Fly. All Rights Reserved.

#include "Components/IndicatorComponent.h"

#include "Subsystems/IndicatorManagerSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Helpers/GeneralHelper.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(IndicatorComponent)

void UIndicatorComponent::BeginPlay()
{
    Super::BeginPlay();

    if (bAddIndicatorOnBegin)
    {
       AddIndicator(bAutoVisible);
    }
}

void UIndicatorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    RemoveIndicator();
    Super::EndPlay(EndPlayReason);
}

void UIndicatorComponent::Deactivate()
{
    SetIndicatorVisibility(false);
    Super::Deactivate();
}

void UIndicatorComponent::PostLoad()
{
    Super::PostLoad();

    if (IndicatorViewModel)
    {
    	const FString ClassName = GetNameSafe(IndicatorViewModel->GetClass());
        UE_LOG(LogUIIndicatorPanel, Log, TEXT("%hs: This %s, IndicatorViewModel Class: %s"), __FUNCTION__, *GetNameSafe(this), *ClassName);
    }
}

void UIndicatorComponent::AddIndicator(bool bMakeVisible)
{
    if (bAddedIndicator)
    {
       return;
    }

    IndicatorViewModel->SetActorAttachedTo(GetOwner());

    if (UIndicatorManagerSubsystem* IndicatorManager = GeneralHelper::GetLocalPlayerSubsystem<UIndicatorManagerSubsystem>(this))
    {
       IndicatorManager->AddIndicator(IndicatorViewModel);
       bAddedIndicator = true;
       Activate(false);
       if (IndicatorViewModel->GetHasVisibilityRange())
       {
          SetIndicatorVisibility(false);
       }
       else
       {
          SetIndicatorVisibility(bMakeVisible);
       }
    }
    else
    {
       UE_LOG(LogUIIndicatorPanel, Warning, TEXT("%s : Indicator could not be added to UIndicatorManagerSubsystem"), *FString(__FUNCTION__));
    }
}

void UIndicatorComponent::RemoveIndicator()
{
    if (bAddedIndicator)
    {
       if (UIndicatorManagerSubsystem* IndicatorManager = GeneralHelper::GetLocalPlayerSubsystem<UIndicatorManagerSubsystem>(this))
       {
          IndicatorManager->RemoveIndicator(IndicatorViewModel);
          bAddedIndicator = false;
       }
       else
       {
          UE_LOG(LogUIIndicatorPanel, Warning, TEXT("%s Can't remove indicator %s because IndicatorManager is nullptr"), *FString(__FUNCTION__), *GetNameSafe(IndicatorViewModel));
       }

       if (IndicatorViewModel)
       {
          IndicatorViewModel->ResetActorAttachedTo();
       }
    }
}

void UIndicatorComponent::SetIndicatorVisibility(bool bInVisibility, EIndicatorVisibilityPriority InPriority)
{
    if (IsActive())
    {
       IndicatorViewModel->SetIndicatorVisibility(bInVisibility, InPriority);
    }
}

void UIndicatorComponent::SetIndicatorClass(TSoftClassPtr<UUserWidget> InIndicatorWidgetClass)
{
    IndicatorViewModel->SetIndicatorClass(MoveTemp(InIndicatorWidgetClass));
}

void UIndicatorComponent::SetIndicatorWorldOffset(const FVector& InWorldOffset) const
{
    IndicatorViewModel->SetWorldPositionOffset(InWorldOffset);
}
