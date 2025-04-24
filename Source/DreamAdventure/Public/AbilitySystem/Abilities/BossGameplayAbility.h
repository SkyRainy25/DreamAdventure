// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/DreamDamageGameplayAbility.h"
#include "BossGameplayAbility.generated.h"

class ADreamProjectile;
class UGameplayEffect;
/**
 * 
 */
UCLASS()
class DREAMADVENTURE_API UBossGameplayAbility : public UDreamDamageGameplayAbility
{
	GENERATED_BODY()
public:


protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride);

	UFUNCTION(BlueprintCallable, Category = "ProjectileBurst")
	void SpawnProjectileBurst(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride, int32 ProjectileNum);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<ADreamProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly)
	int32 NumProjectiles = 5;
};
