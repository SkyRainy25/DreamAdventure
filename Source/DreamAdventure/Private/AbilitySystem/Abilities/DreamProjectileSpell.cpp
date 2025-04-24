// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/DreamProjectileSpell.h"

#include "Actor/DreamProjectile.h"
#include "Interaction/CombatInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "DreamGameplayTags.h"

void UDreamProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);


}

void UDreamProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride)
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

	//// GameplayEffectSpec�� Data�� ä���ִ� �ڵ�.
	//FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
	//EffectContextHandle.SetAbility(this);
	//EffectContextHandle.AddSourceObject(Projectile);
	//TArray<TWeakObjectPtr<AActor>> Actors;
	//Actors.Add(Projectile);
	//EffectContextHandle.AddActors(Actors);
	//FHitResult HitResult;
	//HitResult.Location = ProjectileTargetLocation;
	//EffectContextHandle.AddHitResult(HitResult);

	//const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContextHandle);

	///** �̺κп��� �������� or �������ݿ� ���� ������ ����!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
	//FDreamGameplayTags GameplayTags = FDreamGameplayTags::Get();
	////for (auto& Pair : DamageTypes)
	////{
	////	// GA���� ������ DamageType(Tag) �� Damage(Ŀ�����̺��� ������ ���� ������ ��) ��
	////	// AssignTagSetByCallerMagnitude() �� ����, Tag- Value�� ���� �� �� ����.
	////	const float ScaledDamage = Pair.Value.GetValueAtLevel(GetAbilityLevel());
	////	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, Pair.Key, ScaledDamage);
	////}
	//const float ScaledDamage = Damage.GetValueAtLevel(GetAbilityLevel());
	//UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DamageType, ScaledDamage);
	//// GameplayTag(Key) - Value(Value)  

	//// ADreamProjectile�� DamageEffectSpecHandle�� �����ϸ�, 
	//// TargetASC->ApplyGameplayEffectSpecToSelf(DamageEffectSpecHandle)�� �Ѵ�.
	//Projectile->DamageEffectSpecHandle = SpecHandle;

	// 1. UDreamDamageGameplayAbility::MakeDamageEffectParamsFromClassDefaults() �� ���� GA��  Ability�� ������ ���� �����͸� ����
	// (Projectile Ŭ������ DamageEffectParams �� ���� ������ ä������.
	// 2, DreamProjectile::OnSphereOverlap() ���� �浹�� ApplayDamageEffect�� ���ؼ� ������ TargetActor�� ����
	Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();

	Projectile->FinishSpawning(SpawnTransform);
	
}
