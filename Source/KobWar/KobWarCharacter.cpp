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

//////////////////////////////////////////////////////////////////////////
// AKobWarCharacter

AKobWarCharacter::AKobWarCharacter(const FObjectInitializer& ObjectInitializer) : AClientAuthoritativeCharacter(ObjectInitializer)
{
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
}

//////////////////////////////////////////////////////////////////////////
// Input

void AKobWarCharacter::UpdateSpeed()
{
	if (IsRunning)
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

ECharacterState AKobWarCharacter::GetState()
{
	return CharacterState;
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

	PlayerInputComponent->BindAction("HeavyAttack", IE_Pressed, this, &AKobWarCharacter::AttackHeavyPressed);
	PlayerInputComponent->BindAction("HeavyAttack", IE_Released, this, &AKobWarCharacter::AttackHeavyReleased);

	PlayerInputComponent->BindAction("Block", IE_Pressed, this, &AKobWarCharacter::BlockPressed);
	PlayerInputComponent->BindAction("Block", IE_Released, this, &AKobWarCharacter::BlockReleased);

	PlayerInputComponent->BindAction("Menu", IE_Pressed, this, &AKobWarCharacter::MenuPressed);
	PlayerInputComponent->BindAction("Menu", IE_Released, this, &AKobWarCharacter::MenuReleased);

	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &AKobWarCharacter::SelectPressed);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &AKobWarCharacter::SelectReleased);

	PlayerInputComponent->BindAction("LockOn", IE_Pressed, this, &AKobWarCharacter::LockOnPressed);
	PlayerInputComponent->BindAction("LockOn", IE_Released, this, &AKobWarCharacter::LockOnReleased);
}

void AKobWarCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AKobWarCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AKobWarCharacter::DodgePressed()
{
	OnDodgeButton.Broadcast(true, false);
}

void AKobWarCharacter::DodgeReleased()
{
	OnDodgeButton.Broadcast(false, true);
}

void AKobWarCharacter::ConfirmPressed()
{
	OnConfirmButton.Broadcast(true, false);
}

void AKobWarCharacter::ConfirmReleased()
{
	OnConfirmButton.Broadcast(false, true);
}

void AKobWarCharacter::SelectPressed()
{
	OnSelectButton.Broadcast(true, false);
}

void AKobWarCharacter::SelectReleased()
{
	OnSelectButton.Broadcast(false, true);
}

void AKobWarCharacter::BackPressed()
{
	OnBackButton.Broadcast(true, false);
}

void AKobWarCharacter::BackReleased()
{
	OnBackButton.Broadcast(false, true);

}

void AKobWarCharacter::MenuPressed()
{
	OnMenuButton.Broadcast(true, false);
}

void AKobWarCharacter::MenuReleased()
{
	OnMenuButton.Broadcast(false, true);

}

void AKobWarCharacter::LockOnPressed()
{
	OnLockOnButton.Broadcast(true, false);
}

void AKobWarCharacter::LockOnReleased()
{
	OnLockOnButton.Broadcast(false, true);
}

void AKobWarCharacter::AttackLightPressed()
{
	OnAttackLightButton.Broadcast(true, false);
}

void AKobWarCharacter::AttackLightReleased()
{
	OnAttackLightButton.Broadcast(false, true);
}

void AKobWarCharacter::AttackHeavyPressed()
{
	OnAttackHeavyButton.Broadcast(true, false);
}

void AKobWarCharacter::AttackHeavyReleased()
{
	OnAttackHeavyButton.Broadcast(false, true);
}

void AKobWarCharacter::BlockPressed()
{
	OnBlockButton.Broadcast(true, false);
}

void AKobWarCharacter::BlockReleased()
{
	OnBlockButton.Broadcast(false, true);
}

void AKobWarCharacter::ViewHorizontalMouse(float Value)
{
	if (IsLockedOn && FMath::Abs(Value) >= 0.5f)
	{
		TurnAtRate(Value > 0 ? Value - 0.5f : Value + 0.5f);
	}
	else if (!IsLockedOn)
	{
		TurnAtRate(Value);
	}

	LookDirUpdated();
}

void AKobWarCharacter::ViewVerticalMouse(float Value)
{
	if (IsLockedOn && FMath::Abs(Value) >= 0.5f)
	{
		LookUpAtRate(Value > 0 ? Value - 0.5f : Value + 0.5f);
	}
	else if (!IsLockedOn)
	{
		LookUpAtRate(Value);
	}

	LookDirUpdated();
}

void AKobWarCharacter::ViewHorizontalController(float Value)
{
	if (IsLockedOn && FMath::Abs(Value) >= 0.5f)
	{
		AddControllerYawInput(Value > 0 ? Value - 0.5f : Value + 0.5f);
	}
	else if (!IsLockedOn)
	{
		AddControllerYawInput(Value);
	}

	LookDirUpdated();
}

void AKobWarCharacter::ViewVerticalController(float Value)
{
	if (IsLockedOn && FMath::Abs(Value) >= 0.5f)
	{
		AddControllerPitchInput(Value > 0 ? Value - 0.5f : Value + 0.5f);
	}
	else if (!IsLockedOn)
	{
		AddControllerPitchInput(Value);
	}

	LookDirUpdated();
}

void AKobWarCharacter::LookDirUpdated()
{
	float xVal = FMath::Clamp(GetInputAxisValue("TurnRate") + GetInputAxisValue("Turn"), -1.0f, 1.0f);
	float yVal = FMath::Clamp(- GetInputAxisValue("LookUpRate") - GetInputAxisValue("LookUp"), -1.0f, 1.0f);
	FVector2D vector = FVector2D(xVal, yVal);
	vector.Normalize();

	OnLookDir.Broadcast(vector, FVector2D(xVal, yVal).Size());
}

FVector2D AKobWarCharacter::GetCurrentMovementInput()
{
	float xVal = GetInputAxisValue("MoveForward");
	float yVal = GetInputAxisValue("MoveRight");
	FVector2D vector = FVector2D(xVal, yVal);
	vector.Normalize();

	float dir = FMath::Atan2(vector.Y, vector.X); // relative angle in radians
	dir = FMath::RadiansToDegrees(dir);	// radians to degrees

	return FVector2D(dir, vector.Size());
}

void AKobWarCharacter::MoveForward(float Value)
{
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
