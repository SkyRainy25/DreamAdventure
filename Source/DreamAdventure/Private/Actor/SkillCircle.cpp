// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/SkillCircle.h"
#include "Components/DecalComponent.h"

// Sets default values
ASkillCircle::ASkillCircle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SkillCircleDecal = CreateDefaultSubobject<UDecalComponent>("SkillCircleDecal");
	SkillCircleDecal->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void ASkillCircle::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASkillCircle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

