// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/DreamItem.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/DreamAttributeSet.h"
#include "Components/SphereComponent.h"

#include "Interaction/PlayerInterface.h"

// Sets default values
ADreamItem::ADreamItem()
{
	PrimaryActorTick.bCanEverTick = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("SceneRoot"));

}

// Called when the game starts or when spawned
void ADreamItem::BeginPlay()
{
	Super::BeginPlay();
	
}

void ADreamItem::OnOverlap(AActor* TargetActor)
{
	if (TargetActor->Implements<UPlayerInterface>())
	{
		IPlayerInterface::Execute_AddToItem(TargetActor, ItemTag);
	}
}

