// Copyright People Can Fly. All Rights Reserved."

#include "Helpers/GeneralHelper.h"

namespace GeneralHelper
{
	APlayerController* GetPlayerController(const UObject* WorldContextObject)
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
		if (!ensure(PlayerController))
		{
			return nullptr;
		}

		return PlayerController;
	}
}
