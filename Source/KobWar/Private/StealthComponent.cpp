// Fill out your copyright notice in the Description page of Project Settings.


#include "StealthComponent.h"
#include "ActionControlComponent.h"
#include "KobWar/KobWarCharacter.h"

// Sets default values for this component's properties
UStealthComponent::UStealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UStealthComponent::BeginPlay()
{
	Super::BeginPlay();

	InitOwner();

	BindToTakeHit();

	BindToActionEvent();
}


// Called every frame
void UStealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UStealthComponent::InitOwner()
{
	auto* owner = GetOwner();
	if (!owner)
		return;

	Owner = Cast<AKobWarCharacter>(owner);
}

void UStealthComponent::BindToActionEvent()
{
	if (!Owner)
		return;

	auto* actionControl = Owner->GetActionControl();

	if (!actionControl)
		return;

	actionControl->OnActionBegin.AddDynamic(this, &UStealthComponent::OnActionBegin);
}

void UStealthComponent::OnActionBegin(FName Action)
{
	if (Owner && Owner->IsLocallyControlled() && IsStealthed)
	{
		ToggleStealth(false);
	}
}

void UStealthComponent::ToggleStealth(bool Activate)
{
	if (Owner && Owner->IsLocallyControlled())
	{

		Owner->SetStealthState(Activate);

		if (IsStealthed != Activate)
			OnStealthStateChange.Broadcast(Activate);

		IsStealthed = Activate;
	}
}

void UStealthComponent::OnOwnerDamageTaken(float Damage)
{
	if (Damage >= 5.0f && IsStealthed && Owner->IsLocallyControlled())
	{
		ToggleStealth(false);
	}
}
