// Fill out your copyright notice in the Description page of Project Settings.


#include "SoundPlayerComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
USoundPlayerComponent::USoundPlayerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void USoundPlayerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void USoundPlayerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void USoundPlayerComponent::PlaySoundAtPos(USoundBase* Sound, FVector WorldLoc, float VolumeMultiplier, float PitchMultiplierMin, float PitchMultiplierMax, float StartTime)
{
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, WorldLoc, VolumeMultiplier, FMath::RandRange(PitchMultiplierMin, PitchMultiplierMax), StartTime);
}

