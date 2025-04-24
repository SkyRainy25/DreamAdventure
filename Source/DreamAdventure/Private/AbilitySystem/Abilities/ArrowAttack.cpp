// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/ArrowAttack.h"
#include "DreamAdventure/Public/DreamGameplayTags.h"

#include "AbilitySystem/DreamAbilitySystemLibrary.h"
#include "Actor/DreamProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"

FString UArrowAttack::GetDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	if (Level == 1)
	{
		return FString::Printf(TEXT(
			"<Title> Arrow Attack</>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>마나 ManaCost: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>CoolTime : </><Cooldown>%.1f</>\n\n"

			"<Default> Launched a Arrow to Apply Physical Damage : </>"

			// Damage
			"<Damage>%d</><Default> + </><Damage>'Strength' </>\n\n"),
			Level,
			ManaCost,
			Cooldown,
			ScaledDamage);
	}
	else
	{
		return FString::Printf(TEXT(
			"<Title>Arrow Attack</>\n\n"
			
			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>CoolTime : </><Cooldown>%.1f</>\n\n"

			"<Default> Launched a Arrow to Apply Physical Damage : </>"

			"<Damage>%d</><Default> + </><Damage>'Strength' </>\n\n"),
			Level,
			ManaCost,
			Cooldown,
			ScaledDamage);
	}
}

FString UArrowAttack::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);

	return FString::Printf(TEXT(
		"<Title>NEXT LEVEL: </>\n\n"

		// Level
		"<Small>Level: </><Level>%d</>\n"
		// ManaCost
		"<Small>ManaCost: </><ManaCost>%.1f</>\n"
		// Cooldown
		"<Small>CoolTime : </><Cooldown>%.1f</>\n\n"

		"<Default> Launched a Arrow to Apply Physical Damage : </>"

		"<Damage>%d</><Default> + </><Damage>'Strength' </>\n\n"),

		// Values
		Level,
		ManaCost,
		Cooldown,
		ScaledDamage);
}

void UArrowAttack::SpawnProjectiles(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride, AActor* HomingTarget)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;

	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(
		GetAvatarActorFromActorInfo(),
		SocketTag);
	FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
	if (bOverridePitch) Rotation.Pitch = PitchOverride;

	const FVector Forward = Rotation.Vector();
	const int32 EffectiveNumProjectiles = FMath::Min(NumProjectiles, GetAbilityLevel());
	TArray<FRotator> Rotations = UDreamAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, ProjectileSpread, EffectiveNumProjectiles);

	//NumProjectiles = FMath::Min(MaxNumProjectiles, GetAbilityLevel());
	for (const FRotator& Rot : Rotations)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);
		SpawnTransform.SetRotation(Rot.Quaternion());

		ADreamProjectile* Projectile = GetWorld()->SpawnActorDeferred<ADreamProjectile>(
			ProjectileClass,
			SpawnTransform,
			GetOwningActorFromActorInfo(),
			Cast<APawn>(GetOwningActorFromActorInfo()),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();

		if (HomingTarget && HomingTarget->Implements<UCombatInterface>())
		{
			// Target의 RootCmponent를 가져옴.
			Projectile->ProjectileMovement->HomingTargetComponent = HomingTarget->GetRootComponent();
		}
		else
		{
			// Target이 없을 시, 마우스 커서 위치의 Mesh의 위치 정보를 가져옴.
			Projectile->HomingTargetSceneComponent = NewObject<USceneComponent>(USceneComponent::StaticClass());
			Projectile->HomingTargetSceneComponent->SetWorldLocation(ProjectileTargetLocation);
			Projectile->ProjectileMovement->HomingTargetComponent = Projectile->HomingTargetSceneComponent;
		}

		Projectile->ProjectileMovement->HomingAccelerationMagnitude = FMath::FRandRange(HomingAccelerationMin, HomingAccelerationMax);
		Projectile->ProjectileMovement->bIsHomingProjectile = bLaunchHomingProjectiles;

		Projectile->FinishSpawning(SpawnTransform);
	}
}
