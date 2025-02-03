// Copyright Epic Games, Inc. All Rights Reserved.

#include "KobWarCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "ActionControlComponent.h"
#include "ClimbingComponent.h"
#include <Runtime/Engine/Public/Net/UnrealNetwork.h>


//////////////////////////////////////////////////////////////////////////
// AKobWarCharacter

AKobWarCharacter::AKobWarCharacter(const FObjectInitializer& ObjectInitializer) : AClientAuthoritativeCharacter(ObjectInitializer)
{
	bReplicates = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	UpdateSpeed();

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	ActionControl = CreateDefaultSubobject<UActionControlComponent>(TEXT("ActionControl"));

	LockOnTargetComponent = CreateDefaultSubobject<ULockOnTargSceneComponent>(TEXT("LockOnTargetComponent"));
	LockOnTargetComponent->SetupAttachment(RootComponent);

	ClimbingComponent = CreateDefaultSubobject <UClimbingComponent>(TEXT("ClimbingComp"));

}

void AKobWarCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	FDoRepLifetimeParams sharedParams_SkipOwner;
	sharedParams_SkipOwner.bIsPushBased = true;
	sharedParams_SkipOwner.Condition = COND_SkipOwner;

	FDoRepLifetimeParams sharedParams_NoCond;
	sharedParams_NoCond.bIsPushBased = true;
	sharedParams_NoCond.Condition = COND_None;

	DOREPLIFETIME_WITH_PARAMS(AKobWarCharacter, GenericTeamId, sharedParams_NoCond);
}

void AKobWarCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (NewController->HasAuthority())
	{
		APlayerController* playerController = Cast<APlayerController>(NewController);
		MulticastPossessedByPlayer(playerController);
	}
}

void AKobWarCharacter::MulticastPossessedByPlayer_Implementation(APlayerController* NewController)
{
	OnPlayerPossession.Broadcast(NewController);
}

uint8 AKobWarCharacter::GetCharacterTeamId()
{
	return GenericTeamId;
}

void AKobWarCharacter::SetTeamId(const uint8 NewTeamId)
{
	if (IsLocallyControlled())
	{
		ServerSetTeamId(NewTeamId);
	}
}

void AKobWarCharacter::SetLocalTeamId(const uint8 TeamID)
{
	GenericTeamId = FGenericTeamId(TeamID);
	OnSetTeam.Broadcast(TeamID);
}

void AKobWarCharacter::OnRep_TeamUpdate()
{
	SetLocalTeamId(GenericTeamId);
}

void AKobWarCharacter::ServerSetTeamId_Implementation(const uint8 NewTeamId)
{
	SetLocalTeamId(NewTeamId);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AKobWarCharacter::UpdateSpeed()
{
	if (ActionControl && IsAiming)
	{
		GetCharacterMovement()->MaxWalkSpeed = ActionControl->GetAimMoveSpeed();
	}
	else if (IsRunning)
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseRunSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
}

void AKobWarCharacter::UpdateCameraControlMode(bool ToggleLockedOn)
{
	IsLockedOn = ToggleLockedOn;
}

void AKobWarCharacter::Falling()
{
	Super::Falling();

	OnBeginFalling.Broadcast();
}

void AKobWarCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	OnBeginLanding.Broadcast();
}

bool AKobWarCharacter::ActivateUniqueAction(FActionDataStruct& ActionData)
{
	if (!ActionControl || ActionControl->GetCurrentAction() != ActionControl->NullAction)
	{
		return false;
	}

	return ActionControl->TriggerOtherAction(ActionData);
}

void AKobWarCharacter::UpdateState(TEnumAsByte<ECharacterState> NewState)
{
	auto prevState = CharacterState;
	CharacterState = NewState;
	OnStateChange.Broadcast(CharacterState, prevState);

	switch (NewState)
	{
	case (ECharacterState::Ready):
		UE_LOG(LogTemp, Warning, TEXT("Ready State"), NewState.GetValue());
		break;
	case (ECharacterState::Acting):
		UE_LOG(LogTemp, Warning, TEXT("Acting State"), NewState.GetValue());
		break;
	case (ECharacterState::Falling):
		UE_LOG(LogTemp, Warning, TEXT("Falling State"), NewState.GetValue());
		break;
	}
}

void AKobWarCharacter::SetAimingState(bool Toggle)
{
	IsAiming = Toggle;

	if (ActionControl)
	{
		ActionControl->SetAiming(Toggle);
	}

	UpdateSpeed();
}

ECharacterState AKobWarCharacter::GetState()
{
	return CharacterState;
}

UActionControlComponent* AKobWarCharacter::GetActionControl()
{
	return ActionControl;
}

void AKobWarCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AKobWarCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AKobWarCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &AKobWarCharacter::ViewHorizontalController);
	PlayerInputComponent->BindAxis("TurnRate", this, &AKobWarCharacter::ViewHorizontalMouse);
	PlayerInputComponent->BindAxis("LookUp", this, &AKobWarCharacter::ViewVerticalController);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AKobWarCharacter::ViewVerticalMouse);

	PlayerInputComponent->BindAction("Dodge", IE_Pressed, this, &AKobWarCharacter::DodgePressed);
	PlayerInputComponent->BindAction("Dodge", IE_Released, this, &AKobWarCharacter::DodgeReleased);

	PlayerInputComponent->BindAction("LightAttack", IE_Pressed, this, &AKobWarCharacter::AttackLightPressed);
	PlayerInputComponent->BindAction("LightAttack", IE_Released, this, &AKobWarCharacter::AttackLightReleased);

	PlayerInputComponent->BindAction("HeavyAttack", IE_Pressed, this, &AKobWarCharacter::AttackHeavyPressed);
	PlayerInputComponent->BindAction("HeavyAttack", IE_Released, this, &AKobWarCharacter::AttackHeavyReleased);

	PlayerInputComponent->BindAction("Block", IE_Pressed, this, &AKobWarCharacter::BlockPressed);
	PlayerInputComponent->BindAction("Block", IE_Released, this, &AKobWarCharacter::BlockReleased);

	PlayerInputComponent->BindAction("Menu", IE_Pressed, this, &AKobWarCharacter::MenuPressed);
	PlayerInputComponent->BindAction("Menu", IE_Released, this, &AKobWarCharacter::MenuReleased);

	PlayerInputComponent->BindAction("Scoreboard", IE_Pressed, this, &AKobWarCharacter::ScoreboardPressed);
	PlayerInputComponent->BindAction("Scoreboard", IE_Released, this, &AKobWarCharacter::ScoreboardReleased);

	PlayerInputComponent->BindAction("LockOn", IE_Pressed, this, &AKobWarCharacter::LockOnPressed);
	PlayerInputComponent->BindAction("LockOn", IE_Released, this, &AKobWarCharacter::LockOnReleased);

	PlayerInputComponent->BindAction("WeaponSkill", IE_Pressed, this, &AKobWarCharacter::WeaponSkillPressed);
	PlayerInputComponent->BindAction("WeaponSkill", IE_Released, this, &AKobWarCharacter::WeaponSkillReleased);
}

void AKobWarCharacter::TurnAtRate(float Rate)
{
	if (AreInputsPausedForMenu)
		return;

	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AKobWarCharacter::LookUpAtRate(float Rate)
{
	if (AreInputsPausedForMenu)
		return;

	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AKobWarCharacter::DodgePressed()
{
	if (AreInputsPausedForMenu)
		return;

	OnDodgeButton.Broadcast(true, false);
}

void AKobWarCharacter::DodgeReleased()
{
	if (AreInputsPausedForMenu)
		return;

	OnDodgeButton.Broadcast(false, true);
}

void AKobWarCharacter::ConfirmPressed()
{
	if (AreInputsPausedForMenu)
		return;

	OnConfirmButton.Broadcast(true, false);
}

void AKobWarCharacter::ConfirmReleased()
{
	if (AreInputsPausedForMenu)
		return;

	OnConfirmButton.Broadcast(false, true);
}

void AKobWarCharacter::ScoreboardPressed()
{
	if (AreInputsPausedForMenu)
		return;

	OnScoreboardButton.Broadcast(true, false);
}

void AKobWarCharacter::ScoreboardReleased()
{
	if (AreInputsPausedForMenu)
		return;

	OnScoreboardButton.Broadcast(false, true);
}

void AKobWarCharacter::BackPressed()
{
	if (AreInputsPausedForMenu)
		return;

	OnBackButton.Broadcast(true, false);
}

void AKobWarCharacter::BackReleased()
{
	if (AreInputsPausedForMenu)
		return;

	OnBackButton.Broadcast(false, true);
}

void AKobWarCharacter::MenuPressed()
{
	if (AreInputsPausedForMenu)
		return;

	OnMenuButton.Broadcast(true, false);
}

void AKobWarCharacter::MenuReleased()
{
	if (AreInputsPausedForMenu)
		return;

	OnMenuButton.Broadcast(false, true);
}

void AKobWarCharacter::LockOnPressed()
{
	if (AreInputsPausedForMenu)
		return;

	OnLockOnButton.Broadcast(true, false);
}

void AKobWarCharacter::LockOnReleased()
{
	if (AreInputsPausedForMenu)
		return;

	OnLockOnButton.Broadcast(false, true);
}

void AKobWarCharacter::AttackLightPressed()
{
	if (AreInputsPausedForMenu)
		return;

	OnAttackLightButton.Broadcast(true, false);
}

void AKobWarCharacter::AttackLightReleased()
{
	if (AreInputsPausedForMenu)
		return;

	OnAttackLightButton.Broadcast(false, true);
}

void AKobWarCharacter::AttackHeavyPressed()
{
	if (AreInputsPausedForMenu)
		return;

	OnAttackHeavyButton.Broadcast(true, false);
}

void AKobWarCharacter::AttackHeavyReleased()
{
	if (AreInputsPausedForMenu)
		return;

	OnAttackHeavyButton.Broadcast(false, true);
}

void AKobWarCharacter::BlockPressed()
{
	if (AreInputsPausedForMenu)
		return;

	OnBlockButton.Broadcast(true, false);
}

void AKobWarCharacter::BlockReleased()
{
	if (AreInputsPausedForMenu)
		return;

	OnBlockButton.Broadcast(false, true);
}

void AKobWarCharacter::InteractPressed()
{
	if (AreInputsPausedForMenu)
		return;

	OnInteractButton.Broadcast(true, false);
}

void AKobWarCharacter::InteractReleased()
{
	if (AreInputsPausedForMenu)
		return;

	OnInteractButton.Broadcast(false, true);
}

void AKobWarCharacter::UseItemPressed()
{
	if (AreInputsPausedForMenu)
		return;

	OnUseItemButton.Broadcast(false, true);
}

void AKobWarCharacter::UseItemReleased()
{
	if (AreInputsPausedForMenu)
		return;

	OnUseItemButton.Broadcast(true, false);
}

void AKobWarCharacter::WeaponSkillPressed()
{
	if (AreInputsPausedForMenu)
		return;

	OnWeaponSkill.Broadcast(true, false);
}

void AKobWarCharacter::WeaponSkillReleased()
{
	if (AreInputsPausedForMenu)
		return;

	OnWeaponSkill.Broadcast(false, true);
}

void AKobWarCharacter::ViewHorizontalMouse(float Value)
{
	if (AreInputsPausedForMenu)
		return;

	FVector2D normalizedInput;
	float magnitude = 0.0f;
	LookDirUpdated(normalizedInput, magnitude);

	if (!IsLockedOn)
	{
		float aimingModifier = ActionControl && IsAiming ? 0.25f : 1.0f;

		TurnAtRate(Value * aimingModifier);
	}
}

void AKobWarCharacter::ViewVerticalMouse(float Value)
{
	if (AreInputsPausedForMenu)
		return;

	FVector2D normalizedInput;
	float magnitude = 0.0f;
	LookDirUpdated(normalizedInput, magnitude);

	if (!IsLockedOn)
	{
		float aimingModifier = ActionControl && IsAiming ? 0.25f : 1.0f;

		LookUpAtRate(Value * aimingModifier);
	}
}

void AKobWarCharacter::ViewHorizontalController(float Value)
{
	if (AreInputsPausedForMenu)
		return;

	FVector2D normalizedInput;
	float magnitude = 0.0f;
	LookDirUpdated(normalizedInput, magnitude);

	if (!IsLockedOn)
	{
		float aimingModifier = ActionControl && IsAiming ? 0.25f : 1.0f;

		AddControllerYawInput(Value * aimingModifier);
	}

}

void AKobWarCharacter::ViewVerticalController(float Value)
{
	if (AreInputsPausedForMenu)
		return;

	FVector2D normalizedInput;
	float magnitude = 0.0f;
	LookDirUpdated(normalizedInput, magnitude);

	if (!IsLockedOn)
	{
		float aimingModifier = ActionControl && IsAiming ? 0.25f : 1.0f;

		AddControllerPitchInput(Value * aimingModifier);
	}

}

void AKobWarCharacter::LookDirUpdated(FVector2D& NormalizedInputVector, float& Magnitude)
{
	float xVal = FMath::Clamp(GetInputAxisValue("TurnRate") + GetInputAxisValue("Turn"), -1.0f, 1.0f);
	float yVal = FMath::Clamp(- GetInputAxisValue("LookUpRate") - GetInputAxisValue("LookUp"), -1.0f, 1.0f);
	FVector2D vector = FVector2D(xVal, yVal);
	Magnitude = vector.Size();
	vector.Normalize();
	NormalizedInputVector = vector;

	OnLookDir.Broadcast(vector, FVector2D(xVal, yVal).Size());
}

FVector2D AKobWarCharacter::GetCurrentMovementInput()
{
	float xVal = GetInputAxisValue("MoveForward");
	float yVal = GetInputAxisValue("MoveRight");
	FVector2D vector = FVector2D(xVal, yVal);
	float magnitude = vector.Size();

	vector.Normalize();

	float dir = FMath::Atan2(vector.Y, vector.X); // relative angle in radians
	dir = FMath::RadiansToDegrees(dir);	// radians to degrees

	return FVector2D(dir, magnitude);
}

FVector2D AKobWarCharacter::GetCurrentViewInput()
{
	float xVal = FMath::Clamp(GetInputAxisValue("TurnRate") + GetInputAxisValue("Turn"), -1.0f, 1.0f);
	float yVal = FMath::Clamp(-GetInputAxisValue("LookUpRate") - GetInputAxisValue("LookUp"), -1.0f, 1.0f);
	FVector2D vector = FVector2D(xVal, yVal);

	return vector;
}

FVector2D AKobWarCharacter::GetActorDirectionalVelocity()
{
	FVector Velocity = GetVelocity();

	FVector Forward = GetActorForwardVector();
	FVector Right = GetActorRightVector();

	float ForwardVelocity = FVector::DotProduct(Velocity, Forward);
	float RightVelocity = FVector::DotProduct(Velocity, Right);

	return FVector2D(ForwardVelocity, RightVelocity);
}

void AKobWarCharacter::SetPausedInputsForMenu(bool Pause)
{
	AreInputsPausedForMenu = Pause;
}

void AKobWarCharacter::MoveForward(float Value)
{
	if (AreInputsPausedForMenu)
		return;

	if (FMath::Abs(Value) > 0.05f)
	{
		OnMoveUp.Broadcast(Value);
	}

	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AKobWarCharacter::MoveRight(float Value)
{
	if (AreInputsPausedForMenu)
		return;

	if (FMath::Abs(Value) > 0.05f)
	{
		OnMoveRight.Broadcast(Value);
	}

	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
