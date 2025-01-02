// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ClientAuthoritativePlayerController.h"
#include "GamePlayerController.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class KOBWAR_API AGamePlayerController : public AClientAuthoritativePlayerController
{
	GENERATED_BODY()
	
};
