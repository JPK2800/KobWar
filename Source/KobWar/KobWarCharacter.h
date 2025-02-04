// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ClientAuthoritativeCharacter.h"
#include "LockOnTargSceneComponent.h"
#include "LockOnComponent.h"
#include "GenericTeamAgentInterface.h"
#include "KobWarCharacter.generated.h"

#pragma region Forward Declarations

class UActionControlComponent;
struct FActionDataStruct;

class UClimbingComponent;

#pragma endregion

UENUM(BlueprintType)
enum ECharacterState
{
	Undefined = 0		UMETA(DisplayName = "Undefined"),
	Ready = 1			UMETA(DisplayName = "Ready"),
	Acting = 2			UMETA(DisplayName = "Acting"),
	Falling = 3			UMETA(DisplayName = "Falling"),
};

#pragma region Possession Delegate Declarations

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerPossession, APlayerController*, PlayerController);

#pragma endregion


#pragma region Input Delegate Declarations

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FConfirmButton, bool, Press, bool, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBackButton, bool, Press, bool, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMenuButton, bool, Press, bool, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FScoreboardButton, bool, Press, bool, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLockOnButton, bool, Press, bool, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAttackLightButton, bool, Press, bool, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAttackHeavyButton, bool, Press, bool, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDodgeButton, bool, Press, bool, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBlockButton, bool, Press, bool, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInteractButton, bool, Press, bool, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FUseItemButton, bool, Press, bool, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWeaponSkill, bool, Press, bool, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMoveUp, float, InputY);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMoveRight, float, InputX);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLookDir, FVector2D, Direction, float, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMoveDir, FVector2D, Direction, float, Value);

#pragma endregion

#pragma region Movement Delegate Declarations

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBeginLanding);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBeginFalling);


#pragma endregion

#pragma region Team Delegate Declarations

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetTeam, uint8, NewTeam);


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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Actions", meta = (AllowPrivateAccess = "true"))
	class ULockOnTargSceneComponent* LockOnTargetComponent;

protected:

#pragma region States

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TEnumAsByte<ECharacterState> CharacterState = ECharacterState::Ready;

#pragma endregion

#pragma region LockOn

	bool IsLockedOn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LockOnCamMoveThreshold = 0.8f;
#pragma endregion

#pragma region Menus

	bool AreInputsPausedForMenu = false;

#pragma endregion


public:

#pragma region Possession Events

	UPROPERTY(BlueprintAssignable)
	FPlayerPossession OnPlayerPossession;

#pragma endregion


#pragma region State Change Events

	UPROPERTY(BlueprintAssignable)
	FStateChange OnStateChange;

	UPROPERTY(BlueprintAssignable)
	FBeginFalling OnBeginFalling;

	UPROPERTY(BlueprintAssignable)
	FBeginLanding OnBeginLanding;

#pragma endregion

#pragma region Input Delegates

	UPROPERTY(BlueprintAssignable)
	FConfirmButton OnConfirmButton;

	UPROPERTY(BlueprintAssignable)
	FBackButton OnBackButton;

	UPROPERTY(BlueprintAssignable)
	FMenuButton OnMenuButton;

	UPROPERTY(BlueprintAssignable)
	FScoreboardButton OnScoreboardButton;

	UPROPERTY(BlueprintAssignable)
	FLockOnButton OnLockOnButton;

	UPROPERTY(BlueprintAssignable)
	FAttackLightButton OnAttackLightButton;

	UPROPERTY(BlueprintAssignable)
	FAttackHeavyButton OnAttackHeavyButton;

	UPROPERTY(BlueprintAssignable)
	FDodgeButton OnDodgeButton;

	UPROPERTY(BlueprintAssignable)
	FBlockButton OnBlockButton;

	UPROPERTY(BlueprintAssignable)
	FInteractButton OnInteractButton;

	UPROPERTY(BlueprintAssignable)
	FUseItemButton OnUseItemButton;

	UPROPERTY(BlueprintAssignable)
	FWeaponSkill OnWeaponSkill;

	UPROPERTY(BlueprintAssignable)
	FLookDir OnLookDir;

	UPROPERTY(BlueprintAssignable)
	FMoveDir OnMoveDir;

	UPROPERTY(BlueprintAssignable)
	FMoveUp OnMoveUp;

	UPROPERTY(BlueprintAssignable)
	FMoveRight OnMoveRight;

#pragma endregion

#pragma region Team Events

	UPROPERTY(BlueprintAssignable)
	FSetTeam OnSetTeam;

#pragma endregion


public:
	AKobWarCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma region Possession

	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPossessedByPlayer(APlayerController* NewController);

#pragma endregion


#pragma region Teams

	UPROPERTY(BlueprintReadWrite, EditAnywhere, ReplicatedUsing = OnRep_TeamUpdate, Meta = (ExposeOnSpawn = true))
	uint8 GenericTeamId = 0;

	UFUNCTION(BlueprintCallable)
	uint8 GetCharacterTeamId();
	
	UFUNCTION(BlueprintCallable)
	void SetTeamId(const uint8 NewTeamId);

	virtual void SetLocalTeamId(const uint8 TeamID);

	UFUNCTION(Server, Reliable)
	void ServerSetTeamId(const uint8 NewTeamId);

	UFUNCTION()
	void OnRep_TeamUpdate();

#pragma endregion

	/* Base walking speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BaseMovementSpeed = 150.0f;

	/* Base running speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BaseRunSpeed = 250.0f;

	/* True when the character is running/sprinting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool IsRunning = false;

	/* True when the character is aiming */
	bool IsAiming = false;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;
	
protected:

	float PrevForwardInput = 0.0f;
	float PrevRightInput = 0.0f;

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
	void ScoreboardPressed();

	/* Called when select is released */
	UFUNCTION()
	void ScoreboardReleased();

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

	/* Called when Interact is pressed */
	UFUNCTION()
	void InteractPressed();

	/* Called when Interact is released */
	UFUNCTION()
	void InteractReleased();

	/* Called when UseItem is pressed */
	UFUNCTION()
	void UseItemPressed();

	/* Called when UseItem is released */
	UFUNCTION()
	void UseItemReleased();

	/* Called when weapon skill is pressed */
	UFUNCTION()
	void WeaponSkillPressed();

	/* Called when weapon skill is released */
	UFUNCTION()
	void WeaponSkillReleased();

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
	void LookDirUpdated(FVector2D& NormalizedInputVector, float& Magnitude);


#pragma endregion

#pragma region Movement

	/* Updates the character speed based on all current data */
	UFUNCTION(BlueprintCallable)
	void UpdateSpeed();

#pragma endregion

public:

#pragma region Inputs

	/* Returns the movement input - value 0 is the input direction angle and value 1 is the magnitude*/
	UFUNCTION(BlueprintCallable)
	FVector2D GetCurrentMovementInput();

	/* Returns the view movement input - X is horizontal and Y is vertical and both range from -1 to 1 */
	UFUNCTION(BlueprintCallable)
	FVector2D GetCurrentViewInput();

	/* Returns the current relative movement velocity - X is forward and Y is right */
	UFUNCTION(BlueprintCallable)
	FVector2D GetActorDirectionalVelocity();

	UFUNCTION(BlueprintCallable)
	void SetPausedInputsForMenu(bool Pause);

#pragma endregion
	
#pragma region States

	void Falling() override;

	void Landed(const FHitResult& Hit) override;

	/* Update the character's logic state */
	UFUNCTION(BlueprintCallable)
	void UpdateState(TEnumAsByte<ECharacterState> NewState);

	UFUNCTION(BlueprintCallable)
	void SetAimingState(bool Toggle);

	/* Getter for the character state */
	UFUNCTION(BlueprintCallable)
	ECharacterState GetState();

	/* Action animation play */
	UFUNCTION(BlueprintImplementableEvent)
	void PlayActionAnimation(UAnimMontage* Animation);

#pragma endregion

#pragma region LockOn

	/* Updated lock state from LockOnComponent */
	void UpdateCameraControlMode(bool ToggleLockedOn);

#pragma endregion

#pragma region ActionControl

	bool ActivateUniqueAction(FActionDataStruct& ActionData);

#pragma endregion


#pragma region Getters

public:

	UActionControlComponent* GetActionControl();

#pragma endregion


protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE class ULockOnTargSceneComponent* GetLockOnTargScene() const { return LockOnTargetComponent; }

};

