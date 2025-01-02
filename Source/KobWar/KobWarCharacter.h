// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ClientAuthoritativeCharacter.h"
#include "KobWarCharacter.generated.h"

class UActionControlComponent;

UENUM(BlueprintType)
enum ECharacterState
{
	Undefined = 0		UMETA(DisplayName = "Undefined"),
	Ready = 1			UMETA(DisplayName = "Ready"),
	Acting = 2			UMETA(DisplayName = "Acting"),
	Falling = 3			UMETA(DisplayName = "Falling"),
};

#pragma region Input Delegate Declarations

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FConfirmButton, bool, Press, bool, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBackButton, bool, Press, bool, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMenuButton, bool, Press, bool, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSelectButton, bool, Press, bool, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLockOnButton, bool, Press, bool, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAttackLightButton, bool, Press, bool, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAttackHeavyButton, bool, Press, bool, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBlockButton, bool, Press, bool, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDodgeButton, bool, Press, bool, Release);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLookDir, FVector2D, Direction, float, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMoveDir, FVector2D, Direction, float, Value);

#pragma endregion

#pragma region Movement Delegate Declarations

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBeginLanding);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBeginFalling);


#pragma endregion

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FStateChange, TEnumAsByte<ECharacterState>, NewState, TEnumAsByte<ECharacterState>, OldState);


UCLASS(config=Game, Blueprintable)
class AKobWarCharacter : public AClientAuthoritativeCharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Actions", meta = (AllowPrivateAccess = "true"))
	class UActionControlComponent* ActionControl;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TEnumAsByte<ECharacterState> CharacterState = ECharacterState::Ready;

public:

	UPROPERTY(BlueprintAssignable)
	FStateChange OnStateChange;

	UPROPERTY(BlueprintAssignable)
	FBeginFalling OnBeginFalling;

	UPROPERTY(BlueprintAssignable)
	FBeginLanding OnBeginLanding;

#pragma region Input Delegates

	UPROPERTY(BlueprintAssignable)
	FConfirmButton OnConfirmButton;

	UPROPERTY(BlueprintAssignable)
	FBackButton OnBackButton;

	UPROPERTY(BlueprintAssignable)
	FMenuButton OnMenuButton;

	UPROPERTY(BlueprintAssignable)
	FSelectButton OnSelectButton;

	UPROPERTY(BlueprintAssignable)
	FLockOnButton OnLockOnButton;

	UPROPERTY(BlueprintAssignable)
	FAttackLightButton OnAttackLightButton;

	UPROPERTY(BlueprintAssignable)
	FAttackHeavyButton OnAttackHeavyButton;

	UPROPERTY(BlueprintAssignable)
	FBlockButton OnBlockButton;

	UPROPERTY(BlueprintAssignable)
	FDodgeButton OnDodgeButton;

	UPROPERTY(BlueprintAssignable)
	FLookDir OnLookDir;

	UPROPERTY(BlueprintAssignable)
	FMoveDir OnMoveDir;

#pragma endregion

public:
	AKobWarCharacter(const FObjectInitializer& ObjectInitializer);

	/* Base walking speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BaseMovementSpeed = 150.0f;

	/* Base running speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BaseRunSpeed = 250.0f;

	/* True when the character is running/sprinting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool IsRunning = false;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;
	

protected:

#pragma region Inputs

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/* Called when dodge is pressed */
	UFUNCTION()
	void DodgePressed();

	/* Called when dodge is released */
	UFUNCTION()
	void DodgeReleased();

	/* Called when confirm is pressed */
	UFUNCTION()
	void ConfirmPressed();

	/* Called when confirm is released */
	UFUNCTION()
	void ConfirmReleased();

	/* Called when select is pressed */
	UFUNCTION()
	void SelectPressed();

	/* Called when select is released */
	UFUNCTION()
	void SelectReleased();

	/* Called when back is pressed */
	UFUNCTION()
	void BackPressed();

	/* Called when back is released */
	UFUNCTION()
	void BackReleased();

	/* Called when menu is pressed */
	UFUNCTION()
	void MenuPressed();

	/* Called when menu is released */
	UFUNCTION()
	void MenuReleased();

	/* Called when lock on is pressed */
	UFUNCTION()
	void LockOnPressed();

	/* Called when lock on is released */
	UFUNCTION()
	void LockOnReleased();

	/* Called when attack-light is pressed */
	UFUNCTION()
	void AttackLightPressed();

	/* Called when attack-light is released */
	UFUNCTION()
	void AttackLightReleased();

	/* Called when attack-heavy is pressed */
	UFUNCTION()
	void AttackHeavyPressed();

	/* Called when attack-heavy is released */
	UFUNCTION()
	void AttackHeavyReleased();

	/* Called when block is pressed */
	UFUNCTION()
	void BlockPressed();

	/* Called when block is released */
	UFUNCTION()
	void BlockReleased();

	/* Called when the view is moved horizontally */
	UFUNCTION()
	void ViewHorizontalMouse(float Value);

	/* Called when the view is moved vertically */
	UFUNCTION()
	void ViewVerticalMouse(float Value);

	/* Called when the view is moving horizontally*/
	UFUNCTION()
	void ViewHorizontalController(float Value);

	/* Called when the view is moving vertically*/
	UFUNCTION()
	void ViewVerticalController(float Value);

	/* Called when look-direction changes */
	void LookDirUpdated();

	/* Called when move-direction changes */
	void MoveDirUpdated(FVector2D Values, float Magnitude);

#pragma endregion

	/* Updates the character speed based on all current data */
	UFUNCTION(BlueprintCallable)
	void UpdateSpeed();

public:

	void Falling() override;

	void Landed(const FHitResult& Hit) override;

	/* Update the character's logic state */
	UFUNCTION(BlueprintCallable)
	void UpdateState(TEnumAsByte<ECharacterState> NewState);

	/* Getter for the character state */
	UFUNCTION(BlueprintCallable)
	ECharacterState GetState();

	/* Action animation play */
	UFUNCTION(BlueprintImplementableEvent)
	void PlayActionAnimation(UAnimMontage* Animation);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

