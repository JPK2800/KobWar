// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StealthComponent.generated.h"

class AKobWarCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStealthStateChange, bool, Toggle);

UCLASS( Blueprintable )
class KOBWAR_API UStealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:

	void InitOwner();

public:

	UFUNCTION(BlueprintCallable)
	void ToggleStealth(bool Activate);

	UFUNCTION(BlueprintImplementableEvent)
	void BindToTakeHit();

	void BindToActionEvent();

	UFUNCTION()
	void OnActionBegin(FName Action);

	UFUNCTION(BlueprintCallable)
	void OnOwnerDamageTaken(float Damage);

protected:

	AKobWarCharacter* Owner;

	bool IsStealthed = false;

	float StealthMoveSpeed = 50.0f;

public:

	UPROPERTY(BlueprintAssignable)
	FStealthStateChange OnStealthStateChange;

};
