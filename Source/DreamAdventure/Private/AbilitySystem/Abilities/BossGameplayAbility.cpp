// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/BossGameplayAbility.h"

#include "Actor/DreamProjectile.h"
#include "Interaction/CombatInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "DreamGameplayTags.h"

void UBossGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UBossGameplayAbility::SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();

	if (!bIsServer)	return;

	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(GetAvatarActorFromActorInfo(), SocketTag);
	FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();

	if (bOverridePitch)
	{
		Rotation.Pitch = PitchOverride;
	}

	FTransform SpawnTransform;
	SpawnTransform.SetLocation(SocketLocation);	// Transform�� Location�� ����.
	SpawnTransform.SetRotation(Rotation.Quaternion());

	ADreamProjectile* Projectile = GetWorld()->SpawnActorDeferred<ADreamProjectile>(
		ProjectileClass,	// ������ �߻�ü Ŭ����
		SpawnTransform,		// ���� ��ġ
		GetOwningActorFromActorInfo(),	// Owner
		Cast<APawn>(GetOwningActorFromActorInfo()),	// Instigator
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);

	//TODO: Give the Projectile a Gameplay Effect Spec for causing Damage.
	const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());

	Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();

	Projectile->FinishSpawning(SpawnTransform);
}

void UBossGameplayAbility::SpawnProjectileBurst(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride, int32 ProjectileNum)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();

	if (!bIsServer)	return;

	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(GetAvatarActorFromActorInfo(), SocketTag);
	FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();

	if (bOverridePitch)
	{
		Rotation.Pitch = PitchOverride;
	}

	FTransform SpawnTransform;
	SpawnTransform.SetLocation(SocketLocation);	// Transform�� Location�� ����.
	SpawnTransform.SetRotation(Rotation.Quaternion());

	for (int32 i = 0; i < NumProjectiles; i++)
	{
		ADreamProjectile* Projectile = GetWorld()->SpawnActorDeferred<ADreamProjectile>(
			ProjectileClass,	// ������ �߻�ü Ŭ����
			SpawnTransform,		// ���� ��ġ
			GetOwningActorFromActorInfo(),	// Owner
			Cast<APawn>(GetOwningActorFromActorInfo()),	// Instigator
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);

		//TODO: Give the Projectile a Gameplay Effect Spec for causing Damage.
		const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());

		Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();

		Projectile->FinishSpawning(SpawnTransform);
	}
}
