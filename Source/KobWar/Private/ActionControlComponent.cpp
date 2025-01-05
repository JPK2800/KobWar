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
	}
}

bool UActionControlComponent::CheckQueueForNewAction()
{
	if (CurrentAction.IsCharging)
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
	// ready to trigger the action now
	switch (QueuedAction)
	{
		case (EQueueActions::LightAttack):
			TriggerLightAttack();
			break;
		case (EQueueActions::HeavyAttack):
			TriggerHeavyAttack();
			break;
		case (EQueueActions::Dodge):
			TriggerDodgeAction();
			break;
		case (EQueueActions::Backstep):
			TriggerBackstepAction();
			break;
		default:
			return;
	}

	ClearActionQueue();	// clear queue just in case
}

// Called every frame
void UActionControlComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UActionControlComponent::TriggerLightAttack()
{
	UE_LOG(LogTemp, Warning, TEXT("UActionControlComponent::TriggerLightAttack"));
	return TriggerActionLogic(LightAttack);
}

bool UActionControlComponent::TriggerHeavyAttack()
{
	UE_LOG(LogTemp, Warning, TEXT("UActionControlComponent::TriggerHeavyAttack"));
	return TriggerActionLogic(HeavyAttack);
}

bool UActionControlComponent::TriggerDodgeAction()
{
	UE_LOG(LogTemp, Warning, TEXT("UActionControlComponent::TriggerDodgeAction"));
	return TriggerActionLogic(DodgeAction);
}

bool UActionControlComponent::TriggerBackstepAction()
{
	UE_LOG(LogTemp, Warning, TEXT("UActionControlComponent::TriggerDodgeAction"));
	return TriggerActionLogic(BackstepAction);
}

bool UActionControlComponent::TriggerStaggerAction()
{
	UE_LOG(LogTemp, Warning, TEXT("UActionControlComponent::TriggerStaggerAction"));
	return TriggerActionLogic(StaggerAction);
}

bool UActionControlComponent::TriggerLandAction()
{
	UE_LOG(LogTemp, Warning, TEXT("UActionControlComponent::TriggerLandAction"));
	return TriggerActionLogic(LandAction);
}

bool UActionControlComponent::TriggerActionLogic(FActionDataStruct ActionData)
{
	UE_LOG(LogTemp, Warning, TEXT("UActionControlComponent::TriggerActionLogic"));

	if (!ActionData.HasChargeAnim && CurrentAction.ActionName.IsEqual(ActionData.ActionName))
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

		if (ActionData.HasChargeAnim && !ActionData.IsCharging && ActionData.ChargeAnimData.IsValidIndex(CurrentActionComboIndex) && ActionData.ChargeAnimData[CurrentActionComboIndex].ActionAnimation)
		{
			playAnim = ActionData.ChargeAnimData[CurrentActionComboIndex].ActionAnimation;
			playEndTime = ActionData.ChargeAnimData[CurrentActionComboIndex].GetAllowEndTime();
			playComboTime = ActionData.ChargeAnimData[CurrentActionComboIndex].GetAllowComboTime();
			eventMap = ActionData.ChargeAnimData[CurrentActionComboIndex].GetEvents();
			ActionData.IsCharging = true;
			isAnimFound = true;
		}
		else if (ActionData.AnimData.IsValidIndex(CurrentActionComboIndex) && ActionData.AnimData[CurrentActionComboIndex].ActionAnimation)
		{
			playAnim = ActionData.AnimData[CurrentActionComboIndex].ActionAnimation;
			playEndTime = ActionData.AnimData[CurrentActionComboIndex].GetAllowEndTime();
			playComboTime = ActionData.AnimData[CurrentActionComboIndex].GetAllowComboTime();
			eventMap = ActionData.AnimData[CurrentActionComboIndex].GetEvents();
			ActionData.IsCharging = false;
			isAnimFound = true;
		}
		else if (ActionData.HasChargeAnim && !ActionData.IsCharging && ActionData.ChargeAnimData.IsValidIndex(0) && ActionData.ChargeAnimData[0].ActionAnimation)
		{
			playAnim = ActionData.ChargeAnimData[0].ActionAnimation;
			playEndTime = ActionData.ChargeAnimData[0].GetAllowEndTime();
			playComboTime = ActionData.ChargeAnimData[0].GetAllowComboTime();
			eventMap = ActionData.ChargeAnimData[0].GetEvents();
			ActionData.IsCharging = true;
			isAnimFound = true;
			CurrentActionComboIndex = 0;
		}
		else if (ActionData.AnimData.IsValidIndex(0) && ActionData.AnimData[0].ActionAnimation)
		{
			playAnim = ActionData.AnimData[0].ActionAnimation;
			playEndTime = ActionData.AnimData[0].GetAllowEndTime();
			playComboTime = ActionData.AnimData[0].GetAllowComboTime();
			eventMap = ActionData.AnimData[0].GetEvents();
			ActionData.IsCharging = false;
			isAnimFound = true;
			CurrentActionComboIndex = 0;
		}
		if (isAnimFound && playAnim != nullptr)
		{
			OwnerCharacter->PlayActionAnimation(playAnim);
			CurrentAction = ActionData;
			IsAllowingComboAction = false;
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
			return true;
		}
	}

	return false;
}

bool UActionControlComponent::TriggerChargeComboCurrentAction(bool ForceOnTimeout, bool IsButtonReleased)
{
	if (CurrentAction.IsCharging && IsAllowingComboAction && (ForceOnTimeout || IsButtonReleased))
	{
		bool result = TriggerActionLogic(CurrentAction);
		return result;
	}

	return false;
}

void UActionControlComponent::AllowComboAction()
{
	IsAllowingComboAction = true;

	if (!CheckQueueForNewAction())
	{
		TriggerChargeComboCurrentAction(false, !CurrentAction.IsHeld);
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
	CurrentAction.IsCharging = false;
	CurrentAction = EmptyAction;

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

bool UActionControlComponent::ActivateOrQueueAction(EQueueActions Action)
{
	if (!OwnerCharacter)
	{
		return false;
	}

	if (OwnerCharacter->GetState() == ECharacterState::Ready || IsAllowingComboAction)
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
		LightAttack.IsHeld = true;
		ActivateOrQueueAction(EQueueActions::LightAttack);
	}

	if (Release)
	{
		LightAttack.IsHeld = false;
		if (CurrentAction.ActionName == LightAttack.ActionName)
		{
			TriggerChargeComboCurrentAction(false, true);
		}
	}
}

void UActionControlComponent::ActivateOrQueueHeavyAttack(bool Press, bool Release)
{
	if (Press)
	{
		HeavyAttack.IsHeld = true;
		ActivateOrQueueAction(EQueueActions::HeavyAttack);
	}

	if (Release)
	{
		HeavyAttack.IsHeld = false;
		if (CurrentAction.ActionName == HeavyAttack.ActionName)
		{
			TriggerChargeComboCurrentAction(false, true);
		}
	}
}

void UActionControlComponent::ActivateOrQueueDodge(bool Press, bool Release)
{
	if (Press)
	{
		DodgeAction.IsHeld = true;
		GetWorld()->GetTimerManager().SetTimer(DodgeThresholdTimer, this, &UActionControlComponent::EndDodgeReleaseThreshold, DodgePressReleaseThreshold);
		IsDodgeReleaseThreshold = true;
	}

	if (Release)
	{
		DodgeAction.IsHeld = false;
		if (CurrentAction.ActionName == DodgeAction.ActionName)
		{
			TriggerChargeComboCurrentAction(false, true);
		}
	}

	if (!OwnerCharacter)
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

bool UActionControlComponent::ForceActivateStagger()
{
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

