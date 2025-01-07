// Fill out your copyright notice in the Description page of Project Settings.


#include "LockOnComponent.h"
#include "KobWar/KobWarCharacter.h"
#include "GamePlayerController.h"

// Sets default values for this component's properties
ULockOnComponent::ULockOnComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetComponentTickEnabled(false);

	// ...
}


// Called when the game starts
void ULockOnComponent::BeginPlay()
{
	Super::BeginPlay();

	InitOwnerLink();
}

void ULockOnComponent::InitOwnerLink()
{
	auto* owner = GetOwner();

	OwnerCharacter = Cast<AKobWarCharacter>(owner);
	if (OwnerCharacter)
	{
		OwnerCamComponent = OwnerCharacter->GetFollowCamera();
		DefaultCamRotationRelative = OwnerCamComponent->GetRelativeRotation();
		OwnerSpringArmComponent = OwnerCharacter->GetCameraBoom();

		OwnerCharacter->OnLookDir.AddDynamic(this, &ULockOnComponent::LockOnMoveDir);
		OwnerCharacter->OnLockOnButton.AddDynamic(this, &ULockOnComponent::LockOnPress);

		if (OwnerCharacter->IsLocallyControlled() && OwnerCharacter->IsPlayerControlled())
		{
			OwnerPlayerController = Cast<AGamePlayerController>(OwnerCharacter->GetController());
		}
	}
}

void ULockOnComponent::LockOnPress(bool Press, bool Release)
{
	if (Press)
	{
		if (!LockOnTarget)
		{
			// check for lock on target
			if (auto* foundTarget = LockOnFindTarget())
			{
				SetLockOnTarget(foundTarget);
				return;
			}
		}
		else
		{
			// unlock to target
			SetLockOnTarget(nullptr);
			OwnerCamComponent->SetRelativeRotation(DefaultCamRotationRelative);
			return;
		}
	}
}

ULockOnTargSceneComponent* ULockOnComponent::LockOnFindTarget()
{
	// find only the best target:
	// ideal target is closest to the center of the screen

	auto availableTargets = GetPotentialCharactersForLockOn();

	float bestAngle = 400.0f;
	AKobWarCharacter* bestChar = nullptr;

	TArray<AKobWarCharacter*> keys;
	availableTargets.GetKeys(keys);

	if (keys.Num() <= 0)
	{
		return nullptr;
	}

	for (auto* character : keys)
	{
		FVector2D val = *availableTargets.Find(character);
		if (val.X < bestAngle)
		{
			bestChar = character;
			bestAngle = val.X;
		}
	}

	if (bestChar)
		return bestChar->GetLockOnTargScene();

	return nullptr;
}

bool ULockOnComponent::SetLockOnTarget(ULockOnTargSceneComponent* LockOnTarg)
{
	if (LockOnTarg)
	{
		StartLockSwitchTimer();
		LockOnTarget = LockOnTarg;
		SetComponentTickEnabled(true);
		OnLockedOn.Broadcast(true, LockOnTarg);
		UpdateOwnerLockOnState(true);
		return true;
	}


	SetComponentTickEnabled(false);
	LockOnTarget = nullptr;
	OnLockedOn.Broadcast(false, nullptr);
	UpdateOwnerLockOnState(false);
	return false;
}

void ULockOnComponent::UnlockFromTarget()
{
	LockOnTarget = nullptr;
}

void ULockOnComponent::LockOnMoveDir(FVector2D InputDir, float Value)
{
	if (IsLockSwitchTimerActive || !LockOnTarget)
	{
		return;
	}

	if (Value <= LockOnSwitchThreshold)
	{
		return;
	}

	float inputAngleRadians = FMath::Atan2(-InputDir.Y, InputDir.X) + PI/2;
	float inputAngleDegrees = FMath::RadiansToDegrees(inputAngleRadians);
	if (inputAngleDegrees < 0.0f)
	{
		inputAngleDegrees += 360.0f;
	}

	auto availableTargets = GetPotentialCharactersForLockOn();

	float bestAngle = 2000.0f;
	float bestDist = 8000.0f;
	AKobWarCharacter* bestChar = nullptr;

	TArray<AKobWarCharacter*> keys;
	availableTargets.GetKeys(keys);

	if (keys.Num() <= 0)
	{
		return;
	}

	for (auto* character : keys)
	{
		if (character->GetLockOnTargScene() == LockOnTarget)
		{
			continue;
		}

		FVector2D val = *availableTargets.Find(character);
		float angleDif = inputAngleDegrees - val.Y;
		float posDist = val.X;

		UE_LOG(LogTemp, Warning, TEXT("Char Angle Dist %f   | Char Angle Dir %f    | InputAngle %f   | AngleDif %f"), val.X, val.Y, inputAngleDegrees, angleDif);


		if (angleDif > 180.0f)
		{
			angleDif -= 360.0f;  // Adjust for wrapping around
		}
		else if (angleDif < -180.0f)
		{
			angleDif += 360.0f;  // Adjust for wrapping around
		}
		if (FMath::Abs(angleDif) <= 70.0f && (((FMath::Abs(angleDif) <= 30.0f ) && angleDif < bestAngle) || (posDist < bestDist)))
		{
			bestChar = character;
			bestAngle = angleDif;
			bestDist = posDist;
		}
	}

	if (bestChar)
	{
		SetLockOnTarget(bestChar->GetLockOnTargScene());
		StartLockSwitchTimer();
	}
}

void ULockOnComponent::StartLockSwitchTimer()
{
	GetWorld()->GetTimerManager().SetTimer(LockSwitchTimer, this, &ULockOnComponent::EndLockSwitchTimer, LockSwitchTime);
	IsLockSwitchTimerActive = true;
}

void ULockOnComponent::EndLockSwitchTimer()
{
	IsLockSwitchTimerActive = false;
}

bool ULockOnComponent::IsPosInFov(const FVector ViewPosition, const FRotator ViewRot, const bool IsCurrentlyLockedOn, const FVector CurrentLockOnPos, const FVector TargPosition, const float Fov, float& AngleDistance, float& AngleDirection)
{
	// angle distance
	FVector directionToTarg = (TargPosition - ViewPosition).GetSafeNormal();
	float dotProduct = FVector::DotProduct(ViewRot.Vector(), directionToTarg);
	float angleFovRadians = FMath::Acos(dotProduct);

	//// angle relative to camera view
	float angleFovDegrees = FMath::RadiansToDegrees(angleFovRadians);

	if (angleFovDegrees > Fov / 2.0)
	{
		AngleDistance = 0.0f;
		AngleDirection = 0.0f;
		return false;
	}

	// angle direction
	if (OwnerPlayerController)
	{
		FVector2D screenPos;
		bool targOnScreen = OwnerPlayerController->ProjectWorldLocationToScreen(TargPosition, screenPos);
		if (!targOnScreen)
			return false;

		if (IsCurrentlyLockedOn)
		{
			FVector2D currentLockOnPos;
			bool currentLockOn = OwnerPlayerController->ProjectWorldLocationToScreen(CurrentLockOnPos, currentLockOnPos);
			if (currentLockOn)
			{
				// refer to current lock on target pos on screen

				float angleDirRadians = FMath::Atan2(-(currentLockOnPos.X - screenPos.X), currentLockOnPos.Y - screenPos.Y);
				AngleDirection = FMath::RadiansToDegrees(angleDirRadians);
				AngleDistance = FMath::Abs(currentLockOnPos.X - screenPos.X) + FMath::Abs(currentLockOnPos.Y - screenPos.Y);
				return true;
			}
		}

		// no current lock on target or they are off-screen
		int32 screenX, screenY;
		OwnerPlayerController->GetViewportSize(screenX, screenY);

		float angleDirRadians = FMath::Atan2(-(screenX/2 - screenPos.X), screenY/2 - screenPos.Y);
		AngleDirection = FMath::RadiansToDegrees(angleDirRadians);
		AngleDistance = FMath::Abs(screenX/2 - screenPos.X) + FMath::Abs(screenY/2 - screenPos.Y);
		return true;
		
	}
	else
	{
		// ai units shouldnt be using this function
		AngleDirection = 0.0f;
		AngleDistance = 0.0f;
	}

	return false;
}

TMap<AKobWarCharacter*, FVector2D> ULockOnComponent::GetPotentialCharactersForLockOn()
{
	FVector startPos = OwnerCamComponent->GetComponentLocation();
	float currentTargDist = 2000.0f;
	if (LockOnTarget)
	{
		currentTargDist += UKismetMathLibrary::Vector_Distance(startPos, LockOnTarget->GetComponentLocation());
	}

	FVector endPos = startPos + OwnerCamComponent->GetComponentRotation().Vector() * currentTargDist;

	FVector boxExtent(currentTargDist, 800.0f, 800.0f);

	TArray<FHitResult> hitresults;
	TArray<AActor*> ignoreActors = TArray<AActor*>();
	ignoreActors.Add(OwnerCharacter);
	TArray<ECollisionChannel> queryTypes = TArray<ECollisionChannel>();
	queryTypes.Add(ECollisionChannel::ECC_Pawn);

	UKismetSystemLibrary::BoxTraceMulti(GetWorld(), startPos, endPos, boxExtent, OwnerCamComponent->GetComponentRotation(),
		ETraceTypeQuery::TraceTypeQuery3, false, ignoreActors, IsShowingDebugLines ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None, hitresults, true, FLinearColor::Blue, FLinearColor::Black, 1.0f);

	if (hitresults.Num() <= 0)
		return TMap<AKobWarCharacter*, FVector2D>();

	TMap<AKobWarCharacter*, FVector2D> availableCharacters = TMap<AKobWarCharacter*, FVector2D>();

	for (auto& result : hitresults)
	{
		// verify if the trace is visible for this player
		FVector visioncheckStartPos = startPos;
		FVector visioncheckEndPos;
		AKobWarCharacter* targetChar = Cast<AKobWarCharacter>(result.Actor);

		if (targetChar)
		{
			auto* targLockOnComponent = targetChar->GetLockOnTargScene();
			if (!targLockOnComponent)
				continue;

			FVector targPos = targLockOnComponent->GetComponentLocation();

			float angleDist;
			float angleDir;
			if (!IsPosInFov(startPos, OwnerCamComponent->GetComponentRotation(), LockOnTarget != nullptr, LockOnTarget != nullptr ? LockOnTarget->GetComponentLocation() : FVector::ZeroVector, targPos, OwnerCamComponent->FieldOfView, angleDist, angleDir))
			{
				// not in fov, return
				continue;
			}

			visioncheckEndPos = targPos;
			FHitResult visCheckHitResult;
			TArray<AActor*> ignoreActorsVis = TArray<AActor*>();
			ignoreActorsVis.Add(OwnerCharacter);
			ignoreActorsVis.Add(targetChar);
			bool visResult = UKismetSystemLibrary::LineTraceSingle(GetWorld(), visioncheckStartPos, visioncheckEndPos, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), false, ignoreActorsVis, IsShowingDebugLines ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None, visCheckHitResult, true, FLinearColor::Green, FLinearColor::Red, 1.0f);
			if (visResult)
			{
				// vision blocked
				continue;
			}
			else
			{
				availableCharacters.Add(targetChar, FVector2D(angleDist, angleDir));
			}
		}
	}

	return availableCharacters;
}

void ULockOnComponent::UpdateOwnerLockOnState(bool Toggle)
{
	if (OwnerCharacter)
	{
		OwnerCharacter->UpdateCameraControlMode(Toggle);
	}
}

void ULockOnComponent::InterpCamToTargetStep(float DeltaTime)
{
	if (!LockOnTarget || !OwnerCharacter || !OwnerCamComponent || !OwnerSpringArmComponent)
	{
		return;
	}

	FRotator targSpringArmRot = UKismetMathLibrary::FindLookAtRotation(OwnerSpringArmComponent->GetComponentLocation() - FVector(0.0f, 0.0f, -100.0f), LockOnTarget->GetComponentLocation() - FVector(0.0f, 0.0f, -50.0f));
	if (OwnerCharacter->GetController())
	{
		FRotator controllerRot = OwnerCharacter->GetController()->GetControlRotation();
		FRotator newRot = FMath::RInterpTo(controllerRot, targSpringArmRot, DeltaTime, LockOnRotSpeed);
		OwnerCharacter->GetController()->SetControlRotation(newRot);
	}
}


// Called every frame
void ULockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	InterpCamToTargetStep(DeltaTime);
}

