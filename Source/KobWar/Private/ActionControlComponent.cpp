// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionControlComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UActionControlComponent::UActionControlComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

bool UActionControlComponent::TriggerOtherAction(FActionDataStruct& Data)
{
	if (!OwnerCharacter)
		return false;

	return TriggerActionLogic(Data);
}


// Called when the game starts
void UActionControlComponent::BeginPlay()
{
	Super::BeginPlay();
	
	InitOwnerLink();
}

void UActionControlComponent::InitOwnerLink()
{
	auto* owner = GetOwner();
	OwnerCharacter = Cast<AKobWarCharacter>(owner);
	
	if (OwnerCharacter)
	{
		OwnerCharacter->OnBeginLanding.AddDynamic(this, &UActionControlComponent::Landing);
		OwnerCharacter->OnBeginFalling.AddDynamic(this, &UActionControlComponent::Falling);

		OwnerCharacter->OnAttackLightButton.AddDynamic(this, &UActionControlComponent::ActivateOrQueueLightAttack);
		OwnerCharacter->OnAttackHeavyButton.AddDynamic(this, &UActionControlComponent::ActivateOrQueueHeavyAttack);
		OwnerCharacter->OnDodgeButton.AddDynamic(this, &UActionControlComponent::ActivateOrQueueDodge);
		OwnerCharacter->OnWeaponSkill.AddDynamic(this, &UActionControlComponent::ActivateOrQueueWeaponSkill);

	}
}

bool UActionControlComponent::CheckQueueForNewAction()
{
	if (GetIsChargingAction(CurrentAction))
	{
		return false;
	}

	FDateTime now = FDateTime::Now();
	FTimespan thresholdTime = FTimespan::FromSeconds(1.0f);

	int i = 0;
	while (i < ActionQueue.Num())
	{
		auto& checkAction = ActionQueue[i];

		if (now - checkAction.TimeQueued < thresholdTime)
		{
			// this action is not expired! trigger it and clear the queue
			ActivateAction(checkAction.Action);
			return true;
		}
		i++;
	}
	// no valid action was found - empty the queue just in case
	ClearActionQueue();

	return false;
}

void UActionControlComponent::TriggerStateChange(TEnumAsByte<ECharacterState> NewState)
{
	if (OwnerCharacter)
	{
		OwnerCharacter->UpdateState(NewState);
	}

	ClearActionQueue(); // clear queue on state change just in case
}

void UActionControlComponent::QueueAction(EQueueActions QueueAction)
{
	FActionQueueStruct queueData = FActionQueueStruct();
	queueData.Action = QueueAction;
	queueData.TimeQueued = FDateTime::Now();
	ActionQueue.Add(queueData);
}

void UActionControlComponent::ClearActionQueue()
{
	ActionQueue.Empty();
}

void UActionControlComponent::ActivateAction(EQueueActions QueuedAction)
{
	bool result = false;

	// ready to trigger the action now
	switch (QueuedAction)
	{
	case (EQueueActions::LightAttack):
		result = TriggerLightAttack();
		break;
	case (EQueueActions::HeavyAttack):
		result = TriggerHeavyAttack();
		break;
	case (EQueueActions::Dodge):
		result = TriggerDodgeAction();
		break;
	case (EQueueActions::Backstep):
		result = TriggerBackstepAction();
		break;
	case (EQueueActions::WeaponSkill):
		result = TriggerWeaponSkillAction();
		break;
	case (EQueueActions::RunningAttack):
		result = TriggerRunningAttack();
		break;
	case (EQueueActions::SpecialLight):
		result = TriggerSpecialLightAction();
		break;
	case (EQueueActions::SpecialHeavy):
		result = TriggerSpecialHeavyAction();
		break;
	case (EQueueActions::ClimbUp):
		result = TriggerClimbUp();
		break;
	case (EQueueActions::ClimbDown):
		result = TriggerClimbDown();
		break;
	default:
		return;
	}

	ClearActionQueue();	// clear queue just in case

	if (!result)
	{
		// if there is a current action but we couldn't trigger a new one, it should be force ended now
		if (CurrentAction != NullAction)
		{
			ActionEnd();
		}
	}
}

// Called every frame
void UActionControlComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

FName UActionControlComponent::GetCurrentAction()
{
	return CurrentAction;
}

bool UActionControlComponent::TriggerLightAttack()
{
	UE_LOG(LogTemp, Warning, TEXT("UActionControlComponent::TriggerLightAttack"));

	if (IsClimbing)
		return false;

	if (UseAimWithSpecialHeld && IsAiming)
	{
		return false;
	}
	return TriggerActionLogic(LightAttack);
}

bool UActionControlComponent::TriggerHeavyAttack()
{
	UE_LOG(LogTemp, Warning, TEXT("UActionControlComponent::TriggerHeavyAttack"));

	if (IsClimbing)
		return false;

	if (UseAimWithSpecialHeld && IsAiming)
	{
		return false;
	}
	return TriggerActionLogic(HeavyAttack);
}

bool UActionControlComponent::TriggerDodgeAction()
{
	if (IsClimbing)
		return false;

	if (IsAiming)
		return false;

	UE_LOG(LogTemp, Warning, TEXT("UActionControlComponent::TriggerDodgeAction"));
	return TriggerActionLogic(DodgeAction);
}

bool UActionControlComponent::TriggerWeaponSkillAction()
{
	if (IsClimbing)
		return false;

	if (UseAimWithSpecialHeld)
	{
		// do not use any skill with an aim-state special
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("UActionControlComponent::TriggerWeaponSkillAction"));
	return TriggerActionLogic(WeaponSkillAction);
}

bool UActionControlComponent::TriggerRunningAttack()
{
	if (IsClimbing)
		return false;

	UE_LOG(LogTemp, Warning, TEXT("UActionControlComponent::TriggerRunningAttack"));
	return TriggerActionLogic(RunningAttack);;
}

bool UActionControlComponent::TriggerBackstepAction()
{
	if (IsClimbing)
		return false;

	if (IsAiming)
		return false;

	UE_LOG(LogTemp, Warning, TEXT("UActionControlComponent::TriggerDodgeAction"));
	return TriggerActionLogic(BackstepAction);
}

bool UActionControlComponent::TriggerStaggerAction()
{
	if (IsClimbing)
		return false;

	UE_LOG(LogTemp, Warning, TEXT("UActionControlComponent::TriggerStaggerAction"));
	return TriggerActionLogic(StaggerAction);
}

bool UActionControlComponent::TriggerStagger2Action()
{
	if (IsClimbing)
		return false;

	UE_LOG(LogTemp, Warning, TEXT("UActionControlComponent::TriggerStagger2Action"));
	return TriggerActionLogic(Stagger2Action);
}

bool UActionControlComponent::TriggerLandAction()
{
	if (IsClimbing)
		return false;

	UE_LOG(LogTemp, Warning, TEXT("UActionControlComponent::TriggerLandAction"));
	return TriggerActionLogic(LandAction);
}

bool UActionControlComponent::TriggerActionLogic(FActionDataStruct ActionData)
{
	UE_LOG(LogTemp, Warning, TEXT("UActionControlComponent::TriggerActionLogic"));

	if (!ActionData.HasChargeAnim && CurrentAction.IsEqual(ActionData.ActionName))
	{
		// same as the current action, increment the combo value
		CurrentActionComboIndex++;
	}
	else
	{
		CurrentActionComboIndex = 0;
	}

	if (OwnerCharacter)
	{
		bool isAnimFound = false;
		UAnimMontage* playAnim = nullptr;
		TMap<float, FName> eventMap;
		float playEndTime = -1.0f;
		float playComboTime = -1.0f;

		if (ActionData.HasChargeAnim && !GetIsChargingAction(CurrentAction) && ActionData.ChargeAnimData.IsValidIndex(CurrentActionComboIndex) && ActionData.ChargeAnimData[CurrentActionComboIndex].ActionAnimation)
		{
			playAnim = ActionData.ChargeAnimData[CurrentActionComboIndex].ActionAnimation;
			playEndTime = ActionData.ChargeAnimData[CurrentActionComboIndex].GetAllowEndTime();
			playComboTime = ActionData.ChargeAnimData[CurrentActionComboIndex].GetAllowComboTime();
			eventMap = ActionData.ChargeAnimData[CurrentActionComboIndex].GetEvents();
			SetActionIsCharging(ActionData.ActionName);
			isAnimFound = true;
		}
		else if (ActionData.AnimData.IsValidIndex(CurrentActionComboIndex) && ActionData.AnimData[CurrentActionComboIndex].ActionAnimation)
		{
			playAnim = ActionData.AnimData[CurrentActionComboIndex].ActionAnimation;
			playEndTime = ActionData.AnimData[CurrentActionComboIndex].GetAllowEndTime();
			playComboTime = ActionData.AnimData[CurrentActionComboIndex].GetAllowComboTime();
			eventMap = ActionData.AnimData[CurrentActionComboIndex].GetEvents();
			SetNotChargingActions();
			isAnimFound = true;
		}
		else if (ActionData.HasChargeAnim && !GetIsChargingAction(CurrentAction) && ActionData.ChargeAnimData.IsValidIndex(0) && ActionData.ChargeAnimData[0].ActionAnimation)
		{
			playAnim = ActionData.ChargeAnimData[0].ActionAnimation;
			playEndTime = ActionData.ChargeAnimData[0].GetAllowEndTime();
			playComboTime = ActionData.ChargeAnimData[0].GetAllowComboTime();
			eventMap = ActionData.ChargeAnimData[0].GetEvents();
			SetActionIsCharging(ActionData.ActionName);
			isAnimFound = true;
			CurrentActionComboIndex = 0;
		}
		else if (ActionData.AnimData.IsValidIndex(0) && ActionData.AnimData[0].ActionAnimation)
		{
			playAnim = ActionData.AnimData[0].ActionAnimation;
			playEndTime = ActionData.AnimData[0].GetAllowEndTime();
			playComboTime = ActionData.AnimData[0].GetAllowComboTime();
			eventMap = ActionData.AnimData[0].GetEvents();
			SetNotChargingActions();
			isAnimFound = true;
			CurrentActionComboIndex = 0;
		}
		if (isAnimFound && playAnim != nullptr)
		{
			OwnerCharacter->PlayActionAnimation(playAnim);
			CurrentAction = ActionData.ActionName;
			IsAllowingComboAction = false;
			IsSpecialLightActionReady = false;
			IsSpecialHeavyActionReady = false;
			TriggerStateChange(ECharacterState::Acting);
			if (playComboTime > 0.0f)
			{
				GetWorld()->GetTimerManager().SetTimer(UntilComboTimer, this, &UActionControlComponent::AllowComboAction, playComboTime);
			}
			else if (UntilComboTimer.IsValid())
			{
				GetWorld()->GetTimerManager().ClearTimer(UntilComboTimer);
				UntilComboTimer.Invalidate();
			}
			GetWorld()->GetTimerManager().SetTimer(ActionTimer, this, &UActionControlComponent::ActionEnd, playEndTime > 0.0f ? playEndTime : playAnim->GetPlayLength());
			for (auto& eventEntry : eventMap)
			{
				float timeToTrigger = eventEntry.Key;
				FName eventName = eventEntry.Value;
				FTimerHandle eventTimer = FTimerHandle();
				if (timeToTrigger > 0.0f)
				{
					FTimerDelegate timerDelegate;
					timerDelegate.BindUFunction(this, "HandleAnimationEvent", eventName);
					GetWorld()->GetTimerManager().SetTimer(eventTimer, timerDelegate, timeToTrigger, false);
					EventTimers.Add(eventTimer);
				}
			}

			OnActionBegin.Broadcast(ActionData.ActionName);
			return true;
		}
	}

	return false;
}

bool UActionControlComponent::TriggerChargeComboCurrentAction(bool ForceOnTimeout, bool IsButtonReleased)
{
	if (GetIsChargingAction(CurrentAction) && IsAllowingComboAction && (ForceOnTimeout || IsButtonReleased))
	{
		FActionDataStruct dataFound;
		if (GetChargingAction(dataFound))
		{
			bool result = TriggerActionLogic(dataFound);
			return result;
		}
	}

	return false;
}

bool UActionControlComponent::TriggerSpecialLightAction()
{
	UE_LOG(LogTemp, Warning, TEXT("UActionControlComponent::TriggerSpecialLightAction"));

	if (IsClimbing)
		return false;

	if (IsSpecialLightActionReady || (UseAimWithSpecialHeld && IsAiming))
		return TriggerActionLogic(SpecialLightAction);

	return false;
}

bool UActionControlComponent::TriggerSpecialHeavyAction()
{
	UE_LOG(LogTemp, Warning, TEXT("UActionControlComponent::TriggerSpecialHeavyAction"));

	if (IsClimbing)
		return false;

	if (IsSpecialHeavyActionReady || (UseAimWithSpecialHeld && IsAiming))
		return TriggerActionLogic(SpecialHeavyAction);

	return false;
}

void UActionControlComponent::AllowComboAction()
{
	IsAllowingComboAction = true;

	if (!CheckQueueForNewAction())
	{
		TriggerChargeComboCurrentAction(false, !GetIsActionHeld(CurrentAction));
	}
}

void UActionControlComponent::ActionEnd()
{
	if (CheckQueueForNewAction())
	{
		return;
	}
	if (TriggerChargeComboCurrentAction(true, false))
	{
		return;
	}

	// no queued action

	CurrentActionComboIndex = 0;
	IsAllowingComboAction = false;
	SetNotChargingActions();

	IsSpecialLightActionReady = false;
	IsSpecialHeavyActionReady = false;

	FName prevAction = CurrentAction;
	CurrentAction = FName("?");

	OnActionEnd.Broadcast(prevAction);

	if (auto* movementComp = OwnerCharacter->GetCharacterMovement())
	{
		if (movementComp->IsFalling())
		{
			TriggerStateChange(ECharacterState::Falling);
			return;
		}
	}
	TriggerStateChange(ECharacterState::Ready);

}

bool UActionControlComponent::TriggerClimbUp()
{
	if (!IsClimbing)
		return false;

	UE_LOG(LogTemp, Warning, TEXT("UActionControlComponent::TriggerClimbUp"));
	if (TraceCheckIfClimbingAtTop())
	{
		return TriggerActionLogic(ClimbToTopAction);
	}

	return TriggerActionLogic(ClimbUpAction);
}

bool UActionControlComponent::TriggerClimbUpToTop()
{
	if (!IsClimbing)
		return false;

	UE_LOG(LogTemp, Warning, TEXT("UActionControlComponent::TriggerClimbUpToTop"));
	return TriggerActionLogic(ClimbToTopAction);
}

bool UActionControlComponent::TriggerClimbDown()
{
	if (!IsClimbing)
		return false;

	if (TraceForFloorBelow())
		return false;

	UE_LOG(LogTemp, Warning, TEXT("UActionControlComponent::TriggerClimbDown"));
	return TriggerActionLogic(ClimbDownAction);
}

bool UActionControlComponent::TriggerClimbStagger()
{
	if (!IsClimbing)
		return false;

	UE_LOG(LogTemp, Warning, TEXT("UActionControlComponent::TriggerClimbStagger"));
	return TriggerActionLogic(StartFallingAction);
}

bool UActionControlComponent::TriggerClimbFallGetUp()
{
	if (IsClimbing)
		return false;

	UE_LOG(LogTemp, Warning, TEXT("UActionControlComponent::TriggerClimbFallGetUp"));
	return TriggerActionLogic(GetUpFromClimbFallAction);
}

bool UActionControlComponent::ActivateOrQueueAction(EQueueActions Action)
{
	if (!OwnerCharacter)
	{
		return false;
	}

	if (OwnerCharacter->GetState() == ECharacterState::Ready || IsAllowingComboAction && (!GetIsChargingAction(CurrentAction)))
	{
		// ready to activate
		ActivateAction(Action);
		return true;
	}
	else
	{
		// queue this action
		QueueAction(Action);
		return false;
	}
	return false;
}

void UActionControlComponent::ActivateOrQueueLightAttack(bool Press, bool Release)
{
	if (Press)
	{
		IsLightHeld = true;

		if (OwnerCharacter && (CurrentAction.IsEqual("?") && OwnerCharacter->IsRunning) || (CurrentAction.IsEqual(BackstepAction.ActionName)))
		{
			ActivateOrQueueAction(EQueueActions::RunningAttack);
		}
		else if (OwnerCharacter && (IsSpecialLightActionReady || (UseAimWithSpecialHeld && IsAiming)))
		{
			ActivateOrQueueAction(EQueueActions::SpecialLight);
		}
		else
		{
			ActivateOrQueueAction(EQueueActions::LightAttack);
		}
	}

	if (Release)
	{
		IsLightHeld = false;
		if (CurrentAction == LightAttack.ActionName)
		{
			TriggerChargeComboCurrentAction(false, true);
		}
	}
}

void UActionControlComponent::ActivateOrQueueHeavyAttack(bool Press, bool Release)
{
	if (Press)
	{
		IsHeavyHeld = true;

		if (OwnerCharacter && (IsSpecialHeavyActionReady || (UseAimWithSpecialHeld && IsAiming)))
		{
			ActivateOrQueueAction(EQueueActions::SpecialHeavy);
		}
		else
		{
			ActivateOrQueueAction(EQueueActions::HeavyAttack);
		}
	}

	if (Release)
	{
		IsHeavyHeld = false;
		if (CurrentAction == HeavyAttack.ActionName)
		{
			TriggerChargeComboCurrentAction(false, true);
		}
	}
}

void UActionControlComponent::ActivateOrQueueDodge(bool Press, bool Release)
{
	if (Press)
	{
		IsDodgeHeld = true;
		GetWorld()->GetTimerManager().SetTimer(DodgeThresholdTimer, this, &UActionControlComponent::EndDodgeReleaseThreshold, DodgePressReleaseThreshold);
		IsDodgeReleaseThreshold = true;
	}

	if (Release)
	{
		IsDodgeHeld = false;
		if (CurrentAction == DodgeAction.ActionName && !IsAiming)
		{
			TriggerChargeComboCurrentAction(false, true);
		}
	}

	if (!OwnerCharacter || IsAiming)
		return;

	auto moveInputs = OwnerCharacter->GetCurrentMovementInput();
	if (moveInputs.Y > BackstepToDodgeThreshold)
	{
		// dodge roll logic

		if (Release && IsDodgeReleaseThreshold)
		{
			ActivateOrQueueAction(EQueueActions::Dodge);
		}
	}
	else
	{
		// backstep logic

		if (Release && IsDodgeReleaseThreshold)
		{
			ActivateOrQueueAction(EQueueActions::Backstep);
		}
	}
}

void UActionControlComponent::ActivateOrQueueWeaponSkill(bool Press, bool Release)
{
	if (Press)
	{
		IsWeaponSkillHeld = true;
		if (UseAimWithSpecialHeld)
		{
			SetAimingHeld(true);
		}
		else
		{
			ActivateOrQueueAction(EQueueActions::WeaponSkill);
		}
	}

	if (Release)
	{
		IsWeaponSkillHeld = false;
		if (UseAimWithSpecialHeld)
		{
			SetAimingHeld(false);
		}
		else
		{
			if (CurrentAction == WeaponSkillAction.ActionName)
			{
				TriggerChargeComboCurrentAction(false, true);
			}
		}
	}
}

void UActionControlComponent::Landing()
{
	if (OwnerCharacter)
	{
		ForceActivateLand();
	}
}

void UActionControlComponent::Falling()
{
	if (OwnerCharacter)
	{
		auto currentState = OwnerCharacter->GetState();
		if (currentState != ECharacterState::Falling)
		{
			TriggerStateChange(ECharacterState::Falling);
		}
	}
}

void UActionControlComponent::EndDodgeReleaseThreshold()
{
	IsDodgeReleaseThreshold = false;
}

void UActionControlComponent::ActivateOrQueueClimbUp(bool Press, bool Release)
{
	if (Press)
	{
		if (OwnerCharacter && IsClimbing)
		{
			ActivateOrQueueAction(EQueueActions::ClimbUp);
		}
	}
}

void UActionControlComponent::ActivateOrQueueClimbDown(bool Press, bool Release)
{
	if (Press)
	{
		if (OwnerCharacter && IsClimbing)
		{
			ActivateOrQueueAction(EQueueActions::ClimbDown);
		}
	}
}

bool UActionControlComponent::ForceActivateStagger()
{
	if (CurrentAction.IsEqual(StaggerAction.ActionName) || CurrentAction.IsEqual(Stagger2Action.ActionName))
	{
		return TriggerStagger2Action();
	}
	return TriggerStaggerAction();
}

bool UActionControlComponent::ForceActivateLand()
{
	return TriggerLandAction();
}

void UActionControlComponent::HandleAnimationEvent(FName EventName)
{
	FString stringEventName = EventName.ToString();
	UE_LOG(LogTemp, Warning, TEXT("Event %s"), *stringEventName);

	OnFireActionEvent.Broadcast(stringEventName);
}

bool UActionControlComponent::GetIsChargingAction(FName Action)
{
	if (LightAttack.ActionName.IsEqual(Action))
	{
		return IsLightCharging;
	}
	else if (HeavyAttack.ActionName.IsEqual(Action))
	{
		return IsHeavyCharging;
	}
	else if (DodgeAction.ActionName.IsEqual(Action))
	{
		return IsDodgeCharging;
	}
	else if (WeaponSkillAction.ActionName.IsEqual(Action))
	{
		return IsWeaponSkillCharging;
	}
	return false;
}

void UActionControlComponent::SetNotChargingActions()
{
	IsLightCharging = false;
	IsHeavyCharging = false;
	IsDodgeCharging = false;
	IsWeaponSkillCharging = false;
}

void UActionControlComponent::SetActionIsCharging(FName Action)
{
	if (LightAttack.ActionName.IsEqual(Action))
	{
		IsLightCharging = true;
	}
	else if (HeavyAttack.ActionName.IsEqual(Action))
	{
		IsHeavyCharging = true;
	}
	else if (DodgeAction.ActionName.IsEqual(Action))
	{
		IsDodgeCharging = true;
	}
	else if (WeaponSkillAction.ActionName.IsEqual(Action))
	{
		IsWeaponSkillCharging = true;
	}
}

bool UActionControlComponent::GetChargingAction(FActionDataStruct& Action)
{
	if (IsLightCharging)
	{
		Action = LightAttack;
		return true;
	}
	if (IsHeavyCharging)
	{
		Action = HeavyAttack;
		return true;
	}
	if (IsDodgeCharging)
	{
		Action = DodgeAction;
		return true;
	}
	if (IsWeaponSkillCharging)
	{
		Action = WeaponSkillAction;
		return true;
	}
	return false;
}

bool UActionControlComponent::GetIsActionHeld(FName Action)
{
	if (LightAttack.ActionName.IsEqual(Action))
	{
		return IsLightHeld;
	}
	else if (HeavyAttack.ActionName.IsEqual(Action))
	{
		return IsHeavyHeld;
	}
	else if (DodgeAction.ActionName.IsEqual(Action))
	{
		return IsDodgeHeld;
	}
	else if (WeaponSkillAction.ActionName.IsEqual(Action))
	{
		return IsWeaponSkillHeld;
	}
	return false;
}

void UActionControlComponent::SetIsReadyForSpecialHeavyAction(bool HeavyActionHeavy)
{
	IsSpecialHeavyActionReady = HeavyActionHeavy;
}

void UActionControlComponent::SetAimingHeld(bool Toggle)
{
	OnToggleAiming.Broadcast(Toggle);
}

void UActionControlComponent::SetAiming(bool Toggle)
{
	IsAiming = Toggle;
}

void UActionControlComponent::SetIsClimbing(bool Toggle)
{
	IsClimbing = Toggle;
}

bool UActionControlComponent::TraceCheckIfClimbingAtTop()
{
	FVector start = OwnerCharacter->GetActorLocation() + FVector(0,0,66); 
	FRotator rot = OwnerCharacter->GetActorRotation();
	FVector forwardVector = rot.Vector(); 

	float TraceDistance = 100.0f;

	FVector end = start + forwardVector * TraceDistance;

	// Perform the line trace.
	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(OwnerCharacter); 

	// Perform the horizontal line trace.
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		start,
		end,
		ECC_Visibility, 
		CollisionParams
	);

	if (bHit)
		DrawDebugLine(GetWorld(), start, end, FColor::Green, false, 1.f, 0, 1.f);
	else 
		DrawDebugLine(GetWorld(), start, end, FColor::Red, false, 1.f, 0, 1.f);


	return !bHit;

}

bool UActionControlComponent::TraceForFloorBelow()
{
	FVector start = OwnerCharacter->GetActorLocation();

	FVector TraceDistance = FVector(0, 0, -45.0f);

	FVector end = start + TraceDistance;

	// Perform the line trace.
	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(OwnerCharacter);

	// Perform the horizontal line trace.
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		start,
		end,
		ECC_Visibility,
		CollisionParams
	);

	if (bHit)
		DrawDebugLine(GetWorld(), start, end, FColor::Green, false, 1.f, 0, 1.f);
	else
		DrawDebugLine(GetWorld(), start, end, FColor::Red, false, 1.f, 0, 1.f);


	return bHit;
}


bool UActionControlComponent::GetAimWithSpecialHeld()
{
	return IsAiming;
}

float UActionControlComponent::GetAimMoveSpeed()
{
	return AimingMovementSpeed;
}

FName UActionControlComponent::GetClimbToTopActionName()
{
	return ClimbToTopAction.ActionName;
}


void UActionControlComponent::SetIsReadyForSpecialLightAction(bool LightActionHeavy)
{
	IsSpecialLightActionReady = LightActionHeavy;
}

