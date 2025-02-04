// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "ClimbingMesh.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class KOBWAR_API AClimbingMesh : public AStaticMeshActor
{
	GENERATED_BODY()
	
public:

	AClimbingMesh();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climbing Mesh")
	USceneComponent* ClimbingMeshSceneComponent;
};
