// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "LockOnTargSceneComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "LockOnComponent.generated.h"

class AKobWarCharacter;
class AGamePlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLockedOn, bool, Active, ULockOnTargSceneComponent*, TargComponent);

UCLASS(Blueprintable)
class KOBWAR_API ULockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULockOnComponent();

	UPROPERTY(BlueprintAssignable)
	FLockedOn OnLockedOn;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void InitOwnerLink();

#pragma region LockOn

	UFUNCTION()
	void LockOnPress(bool Press, bool Release);

	ULockOnTargSceneComponent* LockOnFindTarget();

	bool SetLockOnTarget(ULockOnTargSceneComponent* LockOnTarg);

	void UnlockFromTarget();

	UFUNCTION()
	void LockOnMoveDir(FVector2D InputDir, float Value);

	void StartLockSwitchTimer();

	void EndLockSwitchTimer();

	bool IsPosInFov(const FVector ViewPosition, const FRotator ViewRot, const bool IsCurrentlyLockedOn, const FVector CurrentLockOnPos, const FVector TargPosition, const float Fov, float& AngleDistance, float& AngleDirection);

	TMap<AKobWarCharacter*, FVector2D> GetPotentialCharactersForLockOn();

	void UpdateOwnerLockOnState(bool Toggle);

#pragma endregion

#pragma region Lock-on facing

	void InterpCamToTargetStep(float DeltaTime);


#pragma endregion


	

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:

	AKobWarCharacter* OwnerCharacter;

	UCameraComponent* OwnerCamComponent;

	USpringArmComponent* OwnerSpringArmComponent;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	AGamePlayerController* OwnerPlayerController;

	FRotator DefaultCamRotationRelative;

#pragma region Lock-on

	ULockOnTargSceneComponent* LockOnTarget;

	FTimerHandle LockSwitchTimer;

	bool IsLockSwitchTimerActive = false;

#pragma endregion


public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TraceDebug")
	bool IsShowingDebugLines = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LockOnRotSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LockSwitchTime = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LockOnSwitchThreshold = 0.8f;

};
