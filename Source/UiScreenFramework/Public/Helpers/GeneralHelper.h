#pragma once

#include "Kismet/GameplayStatics.h"

namespace GeneralHelper
{
	template <typename SubsystemClass>
	SubsystemClass& GetLocalPlayerSubsystemChecked(const ULocalPlayer* LocalPlayer)
	{
		checkf(LocalPlayer, TEXT("%hs GetChecked<%s>: The passed LocalPlayer is invalid."), __FUNCTION__, *SubsystemClass::StaticClass()->GetName());

		SubsystemClass* Subsystem = LocalPlayer->GetSubsystem<SubsystemClass>();
		checkf(Subsystem, TEXT("%hs GetChecked<%s>: The requested subsystem is invalid."), __FUNCTION__, *SubsystemClass::StaticClass()->GetName());

		return *Subsystem;
	}

	template <typename SubsystemClass>
	SubsystemClass& GetLocalPlayerSubsystem(const ULocalPlayer* LocalPlayer)
	{
		if (!ensure(LocalPlayer))
		{
			return nullptr;
		}

		SubsystemClass* Subsystem = LocalPlayer->GetSubsystem<SubsystemClass>();

		return Subsystem;
	}

	template <typename SubsystemClass>
	SubsystemClass* GetLocalPlayerSubsystem(const UObject* WorldContextObject)
	{
		if (!ensure(WorldContextObject))
		{
			return nullptr;
		}

		const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
		if (!ensure(PlayerController))
		{
			return nullptr;
		}

		const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
		if (!ensure(PlayerController))
		{
			return nullptr;
		}

		SubsystemClass* Subsystem = LocalPlayer->GetSubsystem<SubsystemClass>();

		return Subsystem;
	}

	template <typename SubsystemClass>
	SubsystemClass& GetLocalPlayerSubsystemChecked(const UObject* WorldContextObject)
	{
		checkf(IsValid(WorldContextObject), TEXT("%hs GetChecked<%s>: WorldContextObject is invalid."), __FUNCTION__, *SubsystemClass::StaticClass()->GetName());

		const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
		checkf(PlayerController, TEXT("%hs GetChecked<%s>: APlayerController is invalid."), __FUNCTION__, *SubsystemClass::StaticClass()->GetName());

		const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
		checkf(LocalPlayer, TEXT("%hs GetChecked<%s>: ULocalPlayer is invalid."), __FUNCTION__, *SubsystemClass::StaticClass()->GetName());

		SubsystemClass* Subsystem = LocalPlayer->GetSubsystem<SubsystemClass>();
		checkf(Subsystem, TEXT("%hs GetChecked<%s>: The requested subsystem is invalid."), __FUNCTION__, *SubsystemClass::StaticClass()->GetName());

		return *Subsystem;
	}
}
