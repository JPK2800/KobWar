// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "KobWarGameMode.generated.h"

UENUM(BlueprintType)
enum ETeam
{
	Spectating = 0		UMETA(DisplayName = "Spectating"),
	Team_1 = 1			UMETA(DisplayName = "Team_1"),
	Team_2 = 2			UMETA(DisplayName = "Team_2"),
	NeutralTeam = 3		UMETA(DisplayName = "NeutralTeam"),
};

USTRUCT(BlueprintType)
struct FPlayableClassStruct
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "ClassData")
	FName CharClassName = FName("Unknown");

	UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "ClassData")
	FName Skeleton = FName("Skeleton");

	UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "ClassData")
	TArray<FName> Equipment = TArray<FName>();
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerConnect, APlayerController*, NewPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerDisconnect, AController*, QuitPlayer);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPlayerSetToTeam, APlayerController*, Player, uint8, TeamId);


UCLASS(minimalapi)
class AKobWarGameMode : public AGameMode
{
	GENERATED_BODY()


public:
	AKobWarGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) override;


	UFUNCTION(BlueprintCallable)
	bool SetPlayerTeam(APlayerController* Player, uint8 Team, bool ForceSwitch);

	UPROPERTY(BlueprintAssignable)
	FPlayerConnect OnPlayerConnect;

	UPROPERTY(BlueprintAssignable)
	FPlayerDisconnect OnPlayerDisconnect;

	UPROPERTY(BlueprintAssignable)
	FPlayerSetToTeam OnPlayerSetToTeam;

private:

	TArray<APlayerController*> PlayerControllers = TArray<APlayerController*>();

	// Only set in team-based modes
	TArray<APlayerController*> PlayerControllers_Team1 = TArray<APlayerController*>();

	// Only set in team-based modes
	TArray<APlayerController*> PlayerControllers_Team2 = TArray<APlayerController*>();


public:

#pragma region Player Count

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "PlayerCount")
	uint8 MaxAmountPlayers = 16;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "PlayerCount")
	uint8 MaxSpectators = 2;

#pragma endregion


#pragma region Classes

	// True if players can choose their classes
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CharClasses")
	bool AllowClassSelection = true;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "CharClasses")
	TArray<FPlayableClassStruct> PlayableClasses = TArray<FPlayableClassStruct>();

#pragma endregion

#pragma region Teams

	// True if this gamemode has teams
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Teams")
	bool IsTeamBased = false;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Teams", meta = (EditCondition = "IsTeamBased"))
	uint8 MaxAmountPlayersPerTeam = 8;

	// Set the ratio of players divided between team 1 and team 2. A value of 0.25 means 25% of players will be on Team 1 and 75% will be on team 2. 
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Teams", meta = (EditCondition = "IsTeamBased"))
	float TeamRatio = 0.5f;

	// When true, a team selection widget will appear at the start
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Teams", meta = (EditCondition = "IsTeamBased"))
	bool AllowTeamSelection = true;

	// When true, the player can open the team selection widget from their menu
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Teams", meta = (EditCondition = "IsTeamBased"))
	bool AllowTeamSwitching = true;

	// When true, the player can only switch teams when the other team has fewer players than the TeamRatio prefers.
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Teams", meta = (EditCondition = "IsTeamBased && AllowTeamSwitching"))
	bool AllowInbalancedTeamSwitching = true;

	// Classes to choose from for Team 2.
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Teams", meta = (EditCondition = "IsTeamBased"))
	TArray<FPlayableClassStruct> OtherTeamPlayableClasses = TArray<FPlayableClassStruct>();

	// True if players should switch sides after each round. For assymetrical modes.
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Teams", meta = (EditCondition = "IsTeamBased"))
	bool SwitchSidesEachRound = false;

#pragma endregion

#pragma region GameStart

	// Time until a new game starts
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "GameStart")
	float PreGameWaitTime = 90.0f;

#pragma endregion


};



