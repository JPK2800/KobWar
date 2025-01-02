// Copyright Epic Games, Inc. All Rights Reserved.

#include "KobWarGameMode.h"
#include "KobWarCharacter.h"
#include "UObject/ConstructorHelpers.h"

AKobWarGameMode::AKobWarGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
