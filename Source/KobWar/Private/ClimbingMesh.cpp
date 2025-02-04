// Fill out your copyright notice in the Description page of Project Settings.


#include "ClimbingMesh.h"

AClimbingMesh::AClimbingMesh()
{
	PrimaryActorTick.bCanEverTick = false;

	ClimbingMeshSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ClimbingMeshSceneComponent"));
	ClimbingMeshSceneComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}
