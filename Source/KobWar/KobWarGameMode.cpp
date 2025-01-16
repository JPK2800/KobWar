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

void AKobWarGameMode::BeginPlay()
{
	Super::BeginPlay();

	InitGameStartTimer();

}

void AKobWarGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	OnPlayerConnect.Broadcast(NewPlayer);
}

void AKobWarGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	OnPlayerDisconnect.Broadcast(Exiting);
}

FString AKobWarGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	return Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
}

void AKobWarGameMode::InitGameStartTimer()
{
	GetWorld()->GetTimerManager().SetTimer(PreGameTimer, this, &AKobWarGameMode::TriggerGameStartEvent, PreGameWaitTime);
}

bool AKobWarGameMode::AddToRespawnQueue(AGamePlayerController* Controller)
{
	RespawnQueue.Add(Controller);
	return true;
}

void AKobWarGameMode::InitRespawnTimer()
{
	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &AKobWarGameMode::TriggerRespawnEvent, RespawnTime, true);
}

//bool AKobWarGameMode::SetPlayerTeam(APlayerController* Player, uint8 Team, bool ForceSwitch = false)
//{
//
//	if (PlayerControllers.Num() == 0)
//	{
//		return false;
//	}
//
//	switch (Team)
//	{
//	case (1):
//
//		if (PlayerControllers_Team1.Num() == 0)
//		{
//			OnPlayerSetToTeam.Broadcast(Player, Team);
//			return true;
//		}
//
//		if ((float)(PlayerControllers_Team1.Num() + 1) / (PlayerControllers.Num()) <= TeamRatio || ForceSwitch)
//		{
//			if (PlayerControllers_Team2.Contains(Player))
//				PlayerControllers_Team2.Remove(Player);
//			PlayerControllers_Team1.Add(Player);
//			OnPlayerSetToTeam.Broadcast(Player, Team);
//			return true;
//		}
//		return false;
//
//	case (2):
//
//		if (PlayerControllers_Team2.Num() == 0) {
//			OnPlayerSetToTeam.Broadcast(Player, Team);
//			return true;
//		}
//
//		if ((float)(PlayerControllers_Team2.Num() + 1) / (PlayerControllers.Num()) <= (1 - TeamRatio) || ForceSwitch)
//		{
//			if (PlayerControllers_Team1.Contains(Player))
//				PlayerControllers_Team1.Remove(Player);
//			PlayerControllers_Team2.Add(Player);
//			OnPlayerSetToTeam.Broadcast(Player, Team);
//			return true;
//		}
//		return false;
//	}
//	return false;
//}
