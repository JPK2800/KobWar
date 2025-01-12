// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ClientAuthoritativePlayerController.h"
#include "GamePlayerController.generated.h"


#pragma region Menu Input Delegate Declarations

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMenuConfirm, bool, Press, bool, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMenuBack, bool, Press, bool, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMenuUp, bool, Press, bool, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMenuDown, bool, Press, bool, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMenuLeft, bool, Press, bool, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMenuRight, bool, Press, bool, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMenuMisc1, bool, Press, bool, Release);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMenuMisc2, bool, Press, bool, Release);

#pragma endregion

UCLASS(Blueprintable)
class KOBWAR_API AGamePlayerController : public AClientAuthoritativePlayerController
{
	GENERATED_BODY()
	
protected:

public:

#pragma region State Change Events

	UPROPERTY(BlueprintAssignable)
	FMenuConfirm OnMenuConfirm;

	UPROPERTY(BlueprintAssignable)
	FMenuBack OnMenuBack;

	UPROPERTY(BlueprintAssignable)
	FMenuUp OnMenuUp;

	UPROPERTY(BlueprintAssignable)
	FMenuDown OnMenuDown;

	UPROPERTY(BlueprintAssignable)
	FMenuLeft OnMenuLeft;

	UPROPERTY(BlueprintAssignable)
	FMenuRight OnMenuRight;

	UPROPERTY(BlueprintAssignable)
	FMenuMisc1 OnMenuMisc1;

	UPROPERTY(BlueprintAssignable)
	FMenuMisc2 OnMenuMisc2;

protected:

	virtual void SetupInputComponent() override;

#pragma region Menu Inputs

	UFUNCTION()
	void MenuConfirmPressed();

	UFUNCTION()
	void MenuConfirmReleased();

	UFUNCTION()
	void MenuBackPressed();

	UFUNCTION()
	void MenuBackReleased();

	UFUNCTION()
	void MenuUpPressed();

	UFUNCTION()
	void MenuUpReleased();

	UFUNCTION()
	void MenuDownPressed();

	UFUNCTION()
	void MenuDownReleased();

	UFUNCTION()
	void MenuLeftPressed();

	UFUNCTION()
	void MenuLeftReleased();

	UFUNCTION()
	void MenuRightPressed();

	UFUNCTION()
	void MenuRightReleased();

	UFUNCTION()
	void MenuMisc1Pressed();

	UFUNCTION()
	void MenuMisc1Released();

	UFUNCTION()
	void MenuMisc2Pressed();

	UFUNCTION()
	void MenuMisc2Released();

#pragma endregion

};
