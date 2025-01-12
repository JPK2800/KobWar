// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlayerController.h"

void AGamePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("MenuConfirm", IE_Pressed, this, &AGamePlayerController::MenuConfirmPressed);
	InputComponent->BindAction("MenuConfirm", IE_Released, this, &AGamePlayerController::MenuConfirmReleased);

	InputComponent->BindAction("MenuBack", IE_Pressed, this, &AGamePlayerController::MenuBackPressed);
	InputComponent->BindAction("MenuBack", IE_Released, this, &AGamePlayerController::MenuBackReleased);

	InputComponent->BindAction("MenuUp", IE_Pressed, this, &AGamePlayerController::MenuUpPressed);
	InputComponent->BindAction("MenuUp", IE_Released, this, &AGamePlayerController::MenuUpReleased);
	InputComponent->BindAction("MenuDown", IE_Pressed, this, &AGamePlayerController::MenuDownPressed);
	InputComponent->BindAction("MenuDown", IE_Released, this, &AGamePlayerController::MenuDownReleased);
	InputComponent->BindAction("MenuLeft", IE_Pressed, this, &AGamePlayerController::MenuLeftPressed);
	InputComponent->BindAction("MenuLeft", IE_Released, this, &AGamePlayerController::MenuLeftReleased);
	InputComponent->BindAction("MenuRight", IE_Pressed, this, &AGamePlayerController::MenuRightPressed);
	InputComponent->BindAction("MenuRight", IE_Released, this, &AGamePlayerController::MenuRightReleased);

	InputComponent->BindAction("MenuMisc1", IE_Pressed, this, &AGamePlayerController::MenuMisc1Pressed);
	InputComponent->BindAction("MenuMisc1", IE_Released, this, &AGamePlayerController::MenuMisc1Released);

	InputComponent->BindAction("MenuMisc2", IE_Pressed, this, &AGamePlayerController::MenuMisc2Pressed);
	InputComponent->BindAction("MenuMisc2", IE_Released, this, &AGamePlayerController::MenuMisc2Released);
}

void AGamePlayerController::MenuConfirmPressed()
{
	OnMenuConfirm.Broadcast(true, false);
}

void AGamePlayerController::MenuConfirmReleased()
{
	OnMenuConfirm.Broadcast(false, true);
}

void AGamePlayerController::MenuBackPressed()
{
	OnMenuBack.Broadcast(true, false);
}

void AGamePlayerController::MenuBackReleased()
{
	OnMenuBack.Broadcast(false, true);
}

void AGamePlayerController::MenuUpPressed()
{
	OnMenuUp.Broadcast(true, false);
}

void AGamePlayerController::MenuUpReleased()
{
	OnMenuUp.Broadcast(false, true);
}

void AGamePlayerController::MenuDownPressed()
{
	OnMenuDown.Broadcast(true, false);
}

void AGamePlayerController::MenuDownReleased()
{
	OnMenuDown.Broadcast(false, true);

}

void AGamePlayerController::MenuLeftPressed()
{
	OnMenuLeft.Broadcast(true, false);
}

void AGamePlayerController::MenuLeftReleased()
{
	OnMenuLeft.Broadcast(false, true);
}

void AGamePlayerController::MenuRightPressed()
{
	OnMenuRight.Broadcast(true, false);
}

void AGamePlayerController::MenuRightReleased()
{
	OnMenuRight.Broadcast(false, true);
}

void AGamePlayerController::MenuMisc1Pressed()
{
	OnMenuMisc1.Broadcast(true, false);
}

void AGamePlayerController::MenuMisc1Released()
{
	OnMenuMisc1.Broadcast(false, true);
}

void AGamePlayerController::MenuMisc2Pressed()
{
	OnMenuMisc2.Broadcast(true, false);
}

void AGamePlayerController::MenuMisc2Released()
{
	OnMenuMisc2.Broadcast(false, true);
}
