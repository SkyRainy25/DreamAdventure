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
	SpawnTransform.SetLocation(SocketLocation);	// Transform의 Location을 설정.
	SpawnTransform.SetRotation(Rotation.Quaternion());

	ADreamProjectile* Projectile = GetWorld()->SpawnActorDeferred<ADreamProjectile>(
		ProjectileClass,	// 스폰할 발사체 클래스
		SpawnTransform,		// 스폰 위치
		GetOwningActorFromActorInfo(),	// Owner
		Cast<APawn>(GetOwningActorFromActorInfo()),	// Instigator
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);

	//TODO: Give the Projectile a Gameplay Effect Spec for causing Damage.
	const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());

	//// GameplayEffectSpec의 Data를 채워넣는 코드.
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

	///** 이부분에서 물리공격 or 마법공격에 따른 데미지 설정!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
	//FDreamGameplayTags GameplayTags = FDreamGameplayTags::Get();
	////for (auto& Pair : DamageTypes)
	////{
	////	// GA에서 설정한 DamageType(Tag) 와 Damage(커브테이블에서 설정한 값을 가져온 값) 을
	////	// AssignTagSetByCallerMagnitude() 을 통해, Tag- Value를 매핑 및 값 설정.
	////	const float ScaledDamage = Pair.Value.GetValueAtLevel(GetAbilityLevel());
	////	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, Pair.Key, ScaledDamage);
	////}
	//const float ScaledDamage = Damage.GetValueAtLevel(GetAbilityLevel());
	//UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DamageType, ScaledDamage);
	//// GameplayTag(Key) - Value(Value)  

	//// ADreamProjectile의 DamageEffectSpecHandle을 설정하면, 
	//// TargetASC->ApplyGameplayEffectSpecToSelf(DamageEffectSpecHandle)을 한다.
	//Projectile->DamageEffectSpecHandle = SpecHandle;

	// 1. UDreamDamageGameplayAbility::MakeDamageEffectParamsFromClassDefaults() 를 통해 GA의  Ability의 데미지 관련 데이터를 설정
	// (Projectile 클래스의 DamageEffectParams 에 관련 정보를 채워넣음.
	// 2, DreamProjectile::OnSphereOverlap() 에서 충돌시 ApplayDamageEffect를 통해서 데미지 TargetActor에 적용
	Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();

	Projectile->FinishSpawning(SpawnTransform);
	
}
