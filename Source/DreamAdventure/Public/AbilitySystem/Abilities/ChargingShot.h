// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ArcherChargingSkill.h"
#include "ChargingShot.generated.h"

/**
 * 
 */
UCLASS()
class DREAMADVENTURE_API UChargingShot : public UArcherChargingSkill
{
	GENERATED_BODY()

public:
	virtual FString GetDescription(int32 Level) override;
	virtual FString GetNextLevelDescription(int32 Level) override;
};
