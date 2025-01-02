// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimSequence.h"
#include "KobWar/KobWarCharacter.h"
#include "ActionControlComponent.generated.h"

UENUM(BlueprintType)
enum EQueueActions
{
	UnknownAction = 0		UMETA(DisplayName = "Undefined"),
	LightAttack = 1			UMETA(DisplayName = "LightAttack"),
	HeavyAttack = 2			UMETA(DisplayName = "HeavyAttack"),
	Dodge = 3				UMETA(DisplayName = "Dodge"),
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
};


USTRUCT(BlueprintType)
struct FActionDataStruct
{
	GENERATED_BODY()

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

	bool TriggerStaggerAction();

	bool TriggerLandAction();

	bool TriggerActionLogic(FActionDataStruct ActionData);

	void AllowComboAction();

	void ActionEnd();


#pragma endregion

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintImplementableEvent)
	bool InitializeActionData();

#pragma region Queue or Activate Actions

	UFUNCTION(BlueprintCallable, Category="Action")
	void ActivateOrQueueLightAttack(bool Press, bool Release);

	UFUNCTION(BlueprintCallable, Category = "Action")
	void ActivateOrQueueHeavyAttack(bool Press, bool Release);

	UFUNCTION(BlueprintCallable, Category = "Action")
	void ActivateOrQueueDodge(bool Press, bool Release);

	UFUNCTION()
	void Landing();

	UFUNCTION()
	void Falling();

	void EndDodgeReleaseThreshold();

#pragma endregion

#pragma region Force Activate Actions

	UFUNCTION(BlueprintCallable, Category = "Action")
	bool ForceActivateStagger();

	UFUNCTION(BlueprintCallable, Category = "Action")
	bool ForceActivateLand();

#pragma endregion

	bool ActivateOrQueueAction(EQueueActions Action);

protected:

	AKobWarCharacter* OwnerCharacter = nullptr;

	TArray<FActionQueueStruct> ActionQueue = TArray<FActionQueueStruct>();

	FActionDataStruct CurrentAction;

	FTimerHandle ActionTimer;
	FTimerHandle UntilComboTimer;

	uint8 CurrentActionComboIndex = 0;

	bool IsDodgeReleaseThreshold = false;

	bool IsAllowingComboAction = false;

	FTimerHandle DodgeThresholdTimer;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FActionDataStruct LightAttack;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FActionDataStruct HeavyAttack;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FActionDataStruct DodgeAction;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FActionDataStruct StaggerAction;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FActionDataStruct LandAction;
		
};
