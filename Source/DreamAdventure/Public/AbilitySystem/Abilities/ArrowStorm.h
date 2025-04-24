// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/DreamDamageGameplayAbility.h"
#include "ArrowStorm.generated.h"

class ADreamProjectile;
/**
 * 
 */
UCLASS()
class DREAMADVENTURE_API UArrowStorm : public UDreamDamageGameplayAbility
{
	GENERATED_BODY()

public:
	virtual FString GetDescription(int32 Level) override;
	virtual FString GetNextLevelDescription(int32 Level) override;

	UFUNCTION(BlueprintCallable)
	void SpawnProjectiles(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxNumStorms = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<ADreamProjectile> ProjectileClass;
};
