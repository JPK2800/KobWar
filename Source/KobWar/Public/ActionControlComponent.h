// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimSequence.h"
#include "KobWar/KobWarCharacter.h"
#include "DrawDebugHelpers.h"
#include "ActionControlComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFireActionEvent, FString, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FActionBegin, FName, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FActionEnd, FName, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FToggleAiming, bool, Aiming);



UENUM(BlueprintType)
enum EQueueActions
{
	UnknownAction = 0		UMETA(DisplayName = "Undefined"),
	LightAttack = 1			UMETA(DisplayName = "LightAttack"),
	HeavyAttack = 2			UMETA(DisplayName = "HeavyAttack"),
	Dodge = 3				UMETA(DisplayName = "Dodge"),
	Backstep = 4			UMETA(DisplayName = "Backstep"),
	WeaponSkill = 5			UMETA(DisplayName = "WeaponSkill"),
	RunningAttack = 6		UMETA(DisplayName = "RunningAttack"),
	SpecialLight = 7		UMETA(DisplayName = "SpecialLight"),
	SpecialHeavy = 8		UMETA(DisplayName = "SpecialHeavy"),

	ClimbUp = 20			UMETA(DisplayName = "ClimbUp"),
	ClimbDown = 21			UMETA(DisplayName = "ClimbDown"),

};

USTRUCT(BlueprintType)
struct FAnimationData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActionData")
	UAnimMontage* ActionAnimation = nullptr;

private:

	bool AreTriggersUpdated = false;

	float AllowEndTime = 0.0f;
	float AllowComboTime = 0.0f;

	TMap<float, FName> EventsToFire = TMap<float, FName>();

	bool InitializeTriggers()
	{
		if (ActionAnimation != nullptr)
		{
			const FAnimNotifyTrack notifies = ActionAnimation != nullptr && ActionAnimation->AnimNotifyTracks.IsValidIndex(0) ? ActionAnimation->AnimNotifyTracks[0] : FAnimNotifyTrack();
			for (auto& notif : notifies.Notifies)
			{
				const FName& name = notif->NotifyName;
				const float triggerTime = notif->GetTriggerTime();

				if (name.IsEqual(FName("AllowCombo")))
				{
					AllowComboTime = triggerTime;
				}
				else if (name.IsEqual(FName("AllowEnd")))
				{
					AllowEndTime = triggerTime;
				}
			}

			if (ActionAnimation != nullptr)
			{
				int trackIndex = 1;
				while (ActionAnimation->AnimNotifyTracks.IsValidIndex(trackIndex) && trackIndex <= 2)
				{
					for (auto& notif : ActionAnimation->AnimNotifyTracks[trackIndex].Notifies)
					{
						const FName& name = notif->NotifyName;
						const float triggerTime = notif->GetTriggerTime();

						EventsToFire.Add(TTuple<float, FName>(triggerTime, name));
					}

					trackIndex++;
				}
			}

			AreTriggersUpdated = true;
			return true;
		}
		return false;
	}

public:

	float GetAllowEndTime()
	{
		if (!AreTriggersUpdated)
		{
			InitializeTriggers();
		}

		return AllowEndTime;
	}

	float GetAllowComboTime()
	{
		if (!AreTriggersUpdated)
		{
			InitializeTriggers();
		}

		return AllowComboTime;
	}

	TMap<float, FName> GetEvents()
	{
		if (!AreTriggersUpdated)
		{
			InitializeTriggers();
		}

		return EventsToFire;
	}
};


USTRUCT(BlueprintType)
struct FActionDataStruct
{
	GENERATED_BODY()

	FActionDataStruct() {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActionData")
	FName ActionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActionData")
	TArray<FAnimationData> AnimData = TArray<FAnimationData>();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActionData")
	bool HasChargeAnim = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActionData")
	TArray<FAnimationData> ChargeAnimData = TArray<FAnimationData>();

};

struct FActionQueueStruct
{
	FDateTime TimeQueued;

	EQueueActions Action;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KOBWAR_API UActionControlComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UActionControlComponent();

	bool TriggerOtherAction(FActionDataStruct& Data);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void InitOwnerLink();

	bool CheckQueueForNewAction();

	void TriggerStateChange(TEnumAsByte<ECharacterState> NewState);

#pragma region Queueing and activating actions

	void QueueAction(EQueueActions QueueAction);

	void ClearActionQueue();

	void ActivateAction(EQueueActions QueuedAction);

#pragma endregion

#pragma region Trigger Actions 

	bool TriggerLightAttack();

	bool TriggerHeavyAttack();

	bool TriggerDodgeAction();

	bool TriggerWeaponSkillAction();

	bool TriggerRunningAttack();

	bool TriggerBackstepAction();

	bool TriggerStaggerAction();

	bool TriggerStagger2Action();

	bool TriggerLandAction();

	bool TriggerActionLogic(FActionDataStruct ActionData);

	bool TriggerChargeComboCurrentAction(bool ForceOnTimeout, bool IsButtonReleased);

	bool TriggerSpecialLightAction();

	bool TriggerSpecialHeavyAction();

	void AllowComboAction();

	void ActionEnd();

#pragma region Trigger Climbing Actions

	bool TriggerClimbUp();

	bool TriggerClimbUpToTop();

	bool TriggerClimbDown();

	bool TriggerClimbStagger();

	bool TriggerClimbFallGetUp();

#pragma endregion

#pragma endregion

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintImplementableEvent)
	bool InitializeActionData();

	FName GetCurrentAction();

#pragma region Queue or Activate Actions

	bool ActivateOrQueueAction(EQueueActions Action);

	UFUNCTION(BlueprintCallable, Category="Action")
	void ActivateOrQueueLightAttack(bool Press, bool Release);

	UFUNCTION(BlueprintCallable, Category = "Action")
	void ActivateOrQueueHeavyAttack(bool Press, bool Release);

	UFUNCTION(BlueprintCallable, Category = "Action")
	void ActivateOrQueueDodge(bool Press, bool Release);

	UFUNCTION(BlueprintCallable, Category = "Action")
	void ActivateOrQueueWeaponSkill(bool Press, bool Release);

	UFUNCTION()
	void Landing();

	UFUNCTION()
	void Falling();

	void EndDodgeReleaseThreshold();

#pragma region Climbing Queue or Activate Actions

	UFUNCTION(BlueprintCallable, Category = "Action")
	void ActivateOrQueueClimbUp(bool Press, bool Release);

	UFUNCTION(BlueprintCallable, Category = "Action")
	void ActivateOrQueueClimbDown(bool Press, bool Release);

#pragma endregion


#pragma endregion

#pragma region Force Activate Actions

	UFUNCTION(BlueprintCallable, Category = "Action")
	bool ForceActivateStagger();

	UFUNCTION(BlueprintCallable, Category = "Action")
	bool ForceActivateLand();

#pragma endregion

#pragma region Event handling

	UFUNCTION()
	void HandleAnimationEvent(FName EventName);

#pragma endregion

#pragma region Charge actions

	bool GetIsChargingAction(FName Action);

	void SetNotChargingActions();

	void SetActionIsCharging(FName Action);

	bool GetChargingAction(FActionDataStruct& Action);

	bool GetIsActionHeld(FName Action);

#pragma endregion

#pragma region SpecialActions

	UFUNCTION(BlueprintCallable, Category = "Action")
	void SetIsReadyForSpecialLightAction(bool LightActionReady);

	UFUNCTION(BlueprintCallable, Category = "Action")
	void SetIsReadyForSpecialHeavyAction(bool HeavyActionHeavy);

	void SetAimingHeld(bool Toggle);	// Broadcast the aiming state so the player character can determine if it is time to aim

	void SetAiming(bool Toggle);		// Update when the character is aiming

	void SetIsClimbing(bool Toggle);	// Update when the character is climbing

	bool TraceCheckIfClimbingAtTop();

	bool TraceForFloorBelow();


public:

	UFUNCTION(BlueprintCallable, Category = "Action")
	bool GetAimWithSpecialHeld();

	UFUNCTION(BlueprintCallable, Category = "Action")
	float GetAimMoveSpeed();

	FName GetClimbToTopActionName();

#pragma endregion


protected:

	AKobWarCharacter* OwnerCharacter = nullptr;

	TArray<FActionQueueStruct> ActionQueue = TArray<FActionQueueStruct>();

	TArray<FTimerHandle> EventTimers = TArray<FTimerHandle>();

	FTimerHandle ActionTimer;
	FTimerHandle UntilComboTimer;

	uint8 CurrentActionComboIndex = 0;

	bool IsDodgeReleaseThreshold = false;

	bool IsAllowingComboAction = false;

	bool IsLightHeld = false;

	bool IsLightCharging = false;

	bool IsHeavyHeld = false;

	bool IsHeavyCharging = false;

	bool IsDodgeHeld = false;

	bool IsDodgeCharging = false;

	bool IsWeaponSkillHeld = false;

	bool IsWeaponSkillCharging = false;

	FTimerHandle DodgeThresholdTimer;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actions")
	FActionDataStruct LightAttack;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actions")
	FActionDataStruct HeavyAttack;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actions")
	FActionDataStruct DodgeAction;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actions")
	FActionDataStruct WeaponSkillAction;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actions")
	FActionDataStruct RunningAttack;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actions")
	FActionDataStruct BackstepAction;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actions")
	FActionDataStruct StaggerAction;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actions")
	FActionDataStruct Stagger2Action;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actions")
	FActionDataStruct LandAction;
		
#pragma region Climbing Actions

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actions")
	FActionDataStruct ClimbToTopAction;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actions")
	FActionDataStruct StartFallingAction;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actions")
	FActionDataStruct GetUpFromClimbFallAction;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actions")
	FActionDataStruct ClimbUpAction;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actions")
	FActionDataStruct ClimbDownAction;

#pragma endregion

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inputs")
	float BackstepToDodgeThreshold = 0.5f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inputs")
	float DodgePressReleaseThreshold = 0.20f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actions")
	FActionDataStruct SpecialLightAction;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actions")
	FActionDataStruct SpecialHeavyAction;

	bool IsSpecialLightActionReady = false;

	bool IsSpecialHeavyActionReady = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actions")
	bool UseAimWithSpecialHeld = false;

	bool IsAiming = false;

	bool IsClimbing = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Actions")
	float AimingMovementSpeed = 50.0f;

	FName CurrentAction = FName("?");

	public:

	UPROPERTY(BlueprintAssignable)
	FFireActionEvent OnFireActionEvent;

	UPROPERTY(BlueprintAssignable)
	FActionBegin OnActionBegin;

	UPROPERTY(BlueprintAssignable)
	FActionEnd OnActionEnd;

	UPROPERTY(BlueprintAssignable)
	FToggleAiming OnToggleAiming;

	FName NullAction = FName("?");
};
