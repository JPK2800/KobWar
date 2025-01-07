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

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KOBWAR_API ULockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULockOnComponent();

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

	bool IsPosInFov(FVector ViewPosition, FRotator ViewRot, FVector TargPosition, float Fov, float& AngleDistance, float& AngleDirection);

	TMap<AKobWarCharacter*, FVector2D> GetPotentialCharactersForLockOn();

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

	AGamePlayerController* OwnerPlayerController;

	FRotator DefaultCamRotationRelative;

#pragma region Lock-on

	ULockOnTargSceneComponent* LockOnTarget;

	FTimerHandle LockSwitchTimer;

	bool IsLockSwitchTimerActive = false;

	float LockSwitchTime = 0.35f;

	float LockOnSwitchThreshold = 0.8f;

#pragma endregion


public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TraceDebug")
	bool IsShowingDebugLines = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LockOnRotSpeed = 5.0f;

};
