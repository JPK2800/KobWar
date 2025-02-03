// Fill out your copyright notice in the Description page of Project Settings.


#include "ClimbingComponent.h"

// Sets default values for this component's properties
UClimbingComponent::UClimbingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UClimbingComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UClimbingComponent::InitOwner()
{

	Owner = Cast<AKobWarCharacter>(GetOwner());
	OwnerMovementComp = Owner->GetCharacterMovement();
	ActionControlComp = Owner->GetActionControl();

	if (OwnerMovementComp)
	{
		DefaultGravityScale = OwnerMovementComp->GetGravityZ();
	}
	
}

void UClimbingComponent::MoveDirBinding(bool Bind)
{
	if (Owner)
	{
		if (Bind)
		{
			Owner->OnMoveUp.AddDynamic(this, &UClimbingComponent::ReceiveClimbInput);
		}
		else
		{
			Owner->OnMoveUp.RemoveDynamic(this, &UClimbingComponent::ReceiveClimbInput);
		}
	}
}

void UClimbingComponent::ActionEndBinding(bool Bind)
{
	if (ActionControlComp)
	{
		if (Bind)
		{
			ActionControlComp->OnActionEnd.AddDynamic(this, &UClimbingComponent::ActionEnd);
		}
		else
		{
			ActionControlComp->OnActionEnd.RemoveDynamic(this, &UClimbingComponent::ActionEnd);
		}
	}
}

void UClimbingComponent::LandBinding(bool Bind)
{
	if (Owner)
	{
		if (Bind)
		{
			Owner->OnBeginLanding.AddDynamic(this, &UClimbingComponent::BeginLandedAfterFall);
		}
		else
		{
			Owner->OnBeginLanding.RemoveDynamic(this, &UClimbingComponent::BeginLandedAfterFall);
		}
	}
}

void UClimbingComponent::ToggleClimbingComponent(bool Toggle)
{
	AllowClimbing = Toggle;
}

bool UClimbingComponent::BeginClimbing(AClimbingMesh* ClimbableMesh)
{
	if (!AllowClimbing || !OwnerMovementComp)
		return false;

	if (ClimbableMesh && CurrentClimbState == ClimbState::NotClimbing)
	{
		CurrentClimbMesh = ClimbableMesh;
		CurrentClimbState = ClimbState::BasicClimbing;
		OnClimbStateChange.Broadcast(ClimbState::BasicClimbing);
		OwnerMovementComp->GravityScale = 0.0f;
		MoveDirBinding(true);
		return true;
	}

	return false;
}

void UClimbingComponent::ReceiveClimbInput(float InputY)
{
	if (CurrentClimbState == ClimbState::BasicClimbing)
	{
		FVector climbDir = Owner->GetActorUpVector();
		Owner->AddActorWorldOffset(climbDir * ClimbSpeed);
	}
}

bool UClimbingComponent::BeginTopClimb()
{
	if (CurrentClimbState == ClimbState::BasicClimbing && CurrentClimbMesh)
	{
		bool result =  Owner->ActivateUniqueAction(ClimbToTopAction);

		if (result)
		{
			CurrentClimbState = ClimbState::ClimbingToTop;
			OnClimbStateChange.Broadcast(ClimbState::ClimbingToTop);
			MoveDirBinding(false);
			ActionEndBinding(true);
		}
	}

	return false;
}

void UClimbingComponent::ActionEnd(FName EndedAction)
{
	if (CurrentClimbState == ClimbState::ClimbingToTop)
	{
		ClimbEnd();
	}
	else if (CurrentClimbState == ClimbState::ClimbFalling)
	{
		BeginFalling();
	}
}

void UClimbingComponent::ClimbEnd()
{
	ActionEndBinding(false);
	CurrentClimbState = ClimbState::NotClimbing;
	OnClimbStateChange.Broadcast(ClimbState::NotClimbing);
}

void UClimbingComponent::BeginTakeHitInterruptOnHit()
{
	if (CurrentClimbState == ClimbState::BasicClimbing || CurrentClimbState == ClimbState::ClimbingToTop)
	{
		bool result = Owner->ActivateUniqueAction(StartFallingAction);

		if (result)
		{
			CurrentClimbState = ClimbState::ClimbInterrupted;
			OnClimbStateChange.Broadcast(ClimbState::ClimbInterrupted);
			MoveDirBinding(false);
			ActionEndBinding(true);
		}

	}
}

void UClimbingComponent::BeginFalling()
{
	CurrentClimbState = ClimbState::ClimbFalling;
	OnClimbStateChange.Broadcast(ClimbState::ClimbFalling);
	LandBinding(true);
	ActionEndBinding(false);

}

void UClimbingComponent::BeginLandedAfterFall()
{
	if (CurrentClimbState == ClimbState::ClimbFalling)
	{
		bool result = Owner->ActivateUniqueAction(GetUpAction);

		if (result)
		{
			CurrentClimbState = ClimbState::NotClimbing;
			OnClimbStateChange.Broadcast(ClimbState::NotClimbing);
			LandBinding(false);
		}
	}
}



