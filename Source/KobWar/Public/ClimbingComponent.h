// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ActionControlComponent.h"
#include "KobWar/KobWarCharacter.h"
#include "ClimbingMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ActionControlComponent.h"
#include "ClimbingComponent.generated.h"

UENUM(BlueprintType)
enum ClimbState
{
	NotClimbing = 0,
	BasicClimbing = 1,
	ClimbingToTop = 2,
	ClimbInterrupted = 3,
	ClimbFalling = 4,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FClimbStateChange, TEnumAsByte<ClimbState>, ClimbState);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KOBWAR_API UClimbingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UClimbingComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void InitOwner();

	void MoveDirBinding(bool Bind);

	void ActionEndBinding(bool Bind);

	void LandBinding(bool Bind);

public:	

#pragma region Climbing - Basic process

	void ToggleClimbingComponent(bool Toggle);

	UFUNCTION(BlueprintCallable)
	bool BeginClimbing(AClimbingMesh* ClimbableMesh);

	void ReceiveClimbInput(float InputY);

	UFUNCTION(BlueprintCallable)
	bool BeginTopClimb();

	void ActionEnd(FName EndedAction);

	void ClimbEnd();

#pragma endregion

#pragma region Climbing - Interrupted by taking a hit

	void BeginTakeHitInterruptOnHit();

	void BeginFalling();

	void BeginLandedAfterFall();

#pragma endregion


protected:

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TEnumAsByte<ClimbState> CurrentClimbState = ClimbState::NotClimbing;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Climbing")
	bool AllowClimbing = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Climbing")
	float ClimbSpeed = 5.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Climbing")
	AClimbingMesh* CurrentClimbMesh = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actions")
	FActionDataStruct ClimbToTopAction;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actions")
	FActionDataStruct StartFallingAction;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actions")
	FActionDataStruct GetUpAction;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	AKobWarCharacter* Owner;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UCharacterMovementComponent* OwnerMovementComp;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UActionControlComponent* ActionControlComp;

	float DefaultGravityScale = 1.0f;

public:

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FClimbStateChange OnClimbStateChange;
};
