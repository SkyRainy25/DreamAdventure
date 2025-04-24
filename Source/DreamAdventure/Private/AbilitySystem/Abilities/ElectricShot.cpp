// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/ElectricShot.h"

#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Actor/DreamProjectile.h"
#include "AbilitySystem/DreamAbilitySystemLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"

void UElectricShot::StoreMouseDataInfo(const FHitResult& HitResult)
{
	if (HitResult.bBlockingHit)
	{
		MouseHitLocation = HitResult.ImpactPoint;
		MouseHitActor = HitResult.GetActor();
	}
	else
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}

void UElectricShot::StoreOwnerVariables()
{
	if (CurrentActorInfo)
	{
		OwnerPlayerController = CurrentActorInfo->PlayerController.Get();
		OwnerCharacter = Cast<ACharacter>(CurrentActorInfo->AvatarActor);
	}
}

void UElectricShot::TraceFirstTarget(const FVector& BeamTargetLocation)
{
	check(OwnerCharacter);
	if (OwnerCharacter->Implements<UCombatInterface>())
	{
		if (UStaticMeshComponent* StaticWeapon = ICombatInterface::Execute_GetStaticWeapon(OwnerCharacter))
		{
			TArray<AActor*> ActorsToIgnore;
			ActorsToIgnore.Add(OwnerCharacter);
			FHitResult HitResult;
			const FVector SocketLocation = StaticWeapon->GetSocketLocation(FName("TipSocket"));
			UKismetSystemLibrary::SphereTraceSingle(
				OwnerCharacter,
				SocketLocation,	// Trace Start
				BeamTargetLocation,	// Trace End
				10.f,	// Radius
				TraceTypeQuery1,	//Trace Channel
				false,
				ActorsToIgnore,
				EDrawDebugTrace::None,
				HitResult,
				true);

			if (HitResult.bBlockingHit)
			{
				FirstHitLocation = HitResult.ImpactPoint;
				FirstHitActor = HitResult.GetActor();
			}
		}
	}
	if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(FirstHitActor))
	{
		// Delegate와 콜백함수가 Binding 시켜주는 코드
		if (!CombatInterface->GetOnDeathDelegate().IsAlreadyBound(this, &UElectricShot::PrimaryTargetDied))
		{
			CombatInterface->GetOnDeathDelegate().AddDynamic(this, &UElectricShot::PrimaryTargetDied);
		}
	}
}

void UElectricShot::StoreAdditionalTargets(TArray<AActor*>& OutAdditionalTargets)
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetAvatarActorFromActorInfo());	// 플레이어 제외
	ActorsToIgnore.Add(FirstHitActor);	// 첫 피격 대상 제외

	TArray<AActor*> OverlappingActors;
	UDreamAbilitySystemLibrary::GetLivePlayersWithinRadius(GetAvatarActorFromActorInfo(),
		OverlappingActors,
		ActorsToIgnore,
		1200.f,
		FirstHitActor->GetActorLocation());

	int32 NumAdditionalTargets = FMath::Min(GetAbilityLevel() - 1, MaxNumShockTargets);

	UDreamAbilitySystemLibrary::GetClosestTargets(
		NumAdditionalTargets,
		OverlappingActors,
		OutAdditionalTargets,
		FirstHitActor->GetActorLocation());

	for (AActor* Target : OutAdditionalTargets)
	{
		if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Target))
		{
			if (!CombatInterface->GetOnDeathDelegate().IsAlreadyBound(this, &UElectricShot::AdditionalTargetDied))
			{
				CombatInterface->GetOnDeathDelegate().AddDynamic(this, &UElectricShot::AdditionalTargetDied);
			}
		}
	}
}

void UElectricShot::SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();

	if (!bIsServer)	return;

	// SocketLocation : 화살(Projectile)이 발사될 위치.
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

	const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());

	Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();

	Projectile->FinishSpawning(SpawnTransform);
}

FString UElectricShot::GetDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	if (Level == 1)
	{
		return FString::Printf(TEXT(
			// Title
			"<Title>ElectricShot</>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			"<Default>Emits a beam of lightning from target, "
			"connecting with additional target. </>"

			// Damage
			"<Damage>%d</><Default> lightning damage with"
			" a chance to stun</>"),

			// Values
			Level,
			ManaCost,
			Cooldown,
			ScaledDamage);
	}
	else
	{
		return FString::Printf(TEXT(
			// Title
			"<Title>ElectricShot</>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			// Addition Number of Shock Targets
			"<Default>Emits a beam of lightning, "
			"propagating to %d additional targets nearby, causing </>"

			// Damage
			"<Damage>%d</><Default> lightning damage with"
			" a chance to stun</>"),

			// Values
			Level,
			ManaCost,
			Cooldown,
			FMath::Min(Level, MaxNumShockTargets - 1),
			ScaledDamage);
	}
}

FString UElectricShot::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
		// Title
		"<Title>NEXT LEVEL:</>\n\n"

		// Level
		"<Small>Level: </><Level>%d</>\n"
		// ManaCost
		"<Small>ManaCost: </><ManaCost>%.1f</>\n"
		// Cooldown
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

		// Addition Number of Shock Targets
		"<Default>Emits a beam of lightning from target, "
		"connecting with additional target. </>"

		// Damage
		"<Damage>%d</><Default> lightning damage with"
		" a chance to stun</>"),

		// Values
		Level,
		ManaCost,
		Cooldown,
		FMath::Min(Level, MaxNumShockTargets - 1),
		ScaledDamage);
}
