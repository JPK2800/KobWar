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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMovementModeChange, TEnumAsByte<EMovementMode>, MovementMode);


UCLASS(BlueprintType, Blueprintable)
class KOBWAR_API UClimbingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UClimbingComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:

	void InitOwner();

#pragma region Input and tracing

	void SpecialInputBinding(bool Bind);

	UFUNCTION()
	void SpecialInput(bool Pressed, bool Released);

	bool TraceForClimbableMesh(AClimbingMesh*& FoundMesh);

#pragma endregion

	void MoveDirBinding(bool Bind);

	void ActionEndBinding(bool Bind);

public:	

#pragma region Climbing - Basic process

	void ToggleClimbingComponent(bool Toggle);

	UFUNCTION(BlueprintCallable)
	bool BeginClimbing(AClimbingMesh* ClimbableMesh);

	void InterpToClimbingMesh(float DeltaTime);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateAnimationClimbingState(bool IsClimbing);

	void UpdateOwnerClimbingState(bool IsClimbing);

	UFUNCTION()
	void ReceiveClimbMoveInput(float InputY);

	UFUNCTION()
	void ActionEnd(FName EndedAction);

	void ClimbEnd();

	void SnapOwnerToSurface();

#pragma endregion


protected:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool DebugTrace = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TEnumAsByte<ClimbState> CurrentClimbState = ClimbState::NotClimbing;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Climbing")
	bool AllowClimbing = false;

	bool ClimbHeld = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Climbing")
	float ClimbSpeed = 5.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Climbing")
	AClimbingMesh* CurrentClimbMesh = nullptr;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	AKobWarCharacter* Owner;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UCharacterMovementComponent* OwnerMovementComp;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UActionControlComponent* ActionControlComp;

	float DefaultGravityScale = 1.0f;

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FClimbStateChange OnClimbStateChange;

	UPROPERTY(BlueprintAssignable, BlueprintReadWrite, EditAnywhere)
	FMovementModeChange OnMovementModeChangeEvent;

};
