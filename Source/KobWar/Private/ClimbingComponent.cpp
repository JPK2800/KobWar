// Fill out your copyright notice in the Description page of Project Settings.


#include "ClimbingComponent.h"
#include "DrawDebugHelpers.h" 
#include "Components/CapsuleComponent.h"

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

	InitOwner();

	if (AllowClimbing)
	{
		SpecialInputBinding(true);
	}

	SetComponentTickEnabled(false);
}

void UClimbingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	InterpToClimbingMesh(DeltaTime);
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

void UClimbingComponent::SpecialInputBinding(bool Bind)
{
	if (Owner)
	{
		if (Bind)
		{
			Owner->OnWeaponSkill.AddDynamic(this, &UClimbingComponent::SpecialInput);
		}
		else
		{
			Owner->OnWeaponSkill.RemoveDynamic(this, &UClimbingComponent::SpecialInput);
		}
	}
}

void UClimbingComponent::SpecialInput(bool Pressed, bool Released)
{
	if (ActionControlComp && ActionControlComp->GetCurrentAction() == ActionControlComp->GetClimbToTopActionName())
	{
		return;
	}

	if (Pressed && !Released)
	{
		ClimbHeld = true;
		if (Pressed && !CurrentClimbMesh)
		{
			AClimbingMesh* climbMeshFound;
			if (TraceForClimbableMesh(climbMeshFound))
			{
				BeginClimbing(climbMeshFound);
			}
		}
	}
	else if (Released)
	{
		ClimbHeld = false;
		if (CurrentClimbState == ClimbState::BasicClimbing)
		{
			ClimbEnd();
		}
	}
}

bool UClimbingComponent::TraceForClimbableMesh(AClimbingMesh*& FoundMesh)
{
	if (!Owner)
		return false;

	FVector start = Owner->GetActorLocation(); 
	FVector forwardVector = Owner->GetActorForwardVector();
	FVector end = start + forwardVector * 50.0f;
	FVector boxExtent(64.0f, 64.0f, 24.0f);

	FCollisionQueryParams QueryParams;
	TArray<FHitResult> hitResults;

	bool bHit = GetWorld()->SweepMultiByChannel(
		hitResults,
		start,
		end,
		FQuat::Identity,
		ECC_Visibility,  
		FCollisionShape::MakeBox(boxExtent),
		QueryParams
	);

	if (DebugTrace)
	{
		DrawDebugBox(GetWorld(), start, boxExtent, FQuat::Identity, FColor::Green, false, 1.0f);

	}

	for (const FHitResult& HitResult : hitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor && HitActor->IsA(AClimbingMesh::StaticClass()))
		{
			FoundMesh = Cast<AClimbingMesh>(HitActor); // Found a climbing mesh
			return true;
		}
	}

	FoundMesh = nullptr;
	return false;
}

void UClimbingComponent::MoveDirBinding(bool Bind)
{
	if (Owner)
	{
		if (Bind)
		{
			Owner->OnMoveUp.AddDynamic(this, &UClimbingComponent::ReceiveClimbMoveInput);
		}
		else
		{
			Owner->OnMoveUp.RemoveDynamic(this, &UClimbingComponent::ReceiveClimbMoveInput);
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

void UClimbingComponent::ToggleClimbingComponent(bool Toggle)
{
	if (!AllowClimbing && Toggle)
	{
		SpecialInputBinding(true);
	}
	else if (AllowClimbing && !Toggle)
	{
		SpecialInputBinding(false);
	}

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
		OwnerMovementComp->SetMovementMode(EMovementMode::MOVE_Flying);
		MoveDirBinding(true);
		ActionEndBinding(true);
		UpdateAnimationClimbingState(true);
		UpdateOwnerClimbingState(true);
		SetComponentTickEnabled(true);
		Owner->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Owner->GetCapsuleComponent()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
		return true;
	}

	return false;
}

void UClimbingComponent::InterpToClimbingMesh(float DeltaTime)
{
	if (!CurrentClimbMesh || CurrentClimbState != ClimbState::BasicClimbing || !Owner)
	{
		return;
	}

	FVector currentActorLoc = Owner->GetActorLocation();
	FVector2D currentActorLocXY = FVector2D(currentActorLoc.X, currentActorLoc.Y);

	FVector targetActorLoc = CurrentClimbMesh->ClimbingMeshSceneComponent->GetComponentLocation();
	FVector2D targetLocXY = FVector2D(targetActorLoc.X, targetActorLoc.Y);

	FVector2D newLocXY = FMath::Vector2DInterpTo(currentActorLocXY, targetLocXY, DeltaTime, 5);

	if (FVector2D::Distance(currentActorLocXY, targetLocXY) >= 5.0f)
	{
		Owner->SetActorLocation(FVector(newLocXY.X, newLocXY.Y, currentActorLoc.Z));
	}

	FRotator currentActorRot = Owner->GetActorRotation();
	FRotator targetRot = CurrentClimbMesh->ClimbingMeshSceneComponent->GetComponentRotation();

	FRotator newRot = FMath::RInterpTo(currentActorRot, targetRot, DeltaTime, 5);
	newRot.Pitch = 0;
	newRot.Roll = 0;

	Owner->SetActorRotation(newRot);
}

void UClimbingComponent::UpdateOwnerClimbingState(bool IsClimbing)
{
	if (Owner && ActionControlComp)
	{
		ActionControlComp->SetIsClimbing(IsClimbing);
	}
}


void UClimbingComponent::ReceiveClimbMoveInput(float InputY)
{
	if (ActionControlComp && ActionControlComp->GetCurrentAction() == ActionControlComp->GetClimbToTopActionName())
	{
		return;
	}

	if (CurrentClimbState == ClimbState::BasicClimbing)
	{
		float moveDir = ClimbSpeed * InputY;

		bool result = false;

		if (moveDir >= 0.5)
		{
			ActionControlComp->ActivateOrQueueClimbUp(true, false);
		}
		else if (moveDir <= -0.5)
		{
			ActionControlComp->ActivateOrQueueClimbDown(true, false);
		}
	}
}

void UClimbingComponent::ActionEnd(FName EndedAction)
{
	if (EndedAction == ActionControlComp->GetClimbToTopActionName())
	{
		ClimbEnd();
	}
}

void UClimbingComponent::ClimbEnd()
{
	MoveDirBinding(false);
	ActionEndBinding(false);
	CurrentClimbState = ClimbState::NotClimbing;
	CurrentClimbMesh = nullptr;
	OwnerMovementComp->GravityScale = 2.0f;
	OwnerMovementComp->SetMovementMode(EMovementMode::MOVE_Walking);
	//SnapOwnerToSurface();
	OnClimbStateChange.Broadcast(ClimbState::NotClimbing);
	UpdateAnimationClimbingState(false);
	UpdateOwnerClimbingState(false);
	SetComponentTickEnabled(false);
	Owner->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Owner->GetCapsuleComponent()->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);


}

void UClimbingComponent::SnapOwnerToSurface()
{
	FVector start = Owner->GetActorLocation() + FVector(0,0,50);
	FVector end = start - FVector(0, 0, 300);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, start, end, ECC_Visibility, Params))
	{
		FVector NewLocation = HitResult.Location;
		Owner->SetActorLocation(NewLocation);
	}
}



