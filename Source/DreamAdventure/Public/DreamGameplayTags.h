// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * DreamGameplayTags
 *
 * Singleton containing native Gameplay Tags
 * Native => c++ & blurprint 에서 사용가능.
 */

struct FDreamGameplayTags
{
public:
	static const FDreamGameplayTags& Get() { return GameplayTags; }
	static void InitializeNativeGameplayTags();

	FGameplayTag Attributes_Primary_Strength;
	FGameplayTag Attributes_Primary_Intelligence;
	FGameplayTag Attributes_Primary_Resilience;
	FGameplayTag Attributes_Primary_Vigor;

	FGameplayTag Attributes_Secondary_Armor;
	FGameplayTag Attributes_Secondary_ArmorPenetration;
	FGameplayTag Attributes_Secondary_CriticalHitChance;
	FGameplayTag Attributes_Secondary_CriticalHitDamage;
	FGameplayTag Attributes_Secondary_HealthRegeneration;
	FGameplayTag Attributes_Secondary_ManaRegeneration;
	FGameplayTag Attributes_Secondary_MaxHealth;
	FGameplayTag Attributes_Secondary_MaxMana;

	// Meta Attributes
	FGameplayTag Attributes_Meta_IncomingXP;

	FGameplayTag InputTag_LMB;
	FGameplayTag InputTag_RMB;
	FGameplayTag InputTag_1;
	FGameplayTag InputTag_2;
	FGameplayTag InputTag_3;
	FGameplayTag InputTag_4;
	FGameplayTag InputTag_5;
	FGameplayTag InputTag_6;
	FGameplayTag InputTag_G;
	FGameplayTag InputTag_Passive_1;
	FGameplayTag InputTag_Passive_2;

	// GameplayAbility의 AssginTagSetByCallerMagnitude를 통해 Damage를 설정하기 위한 Tag
	FGameplayTag Damage;

	// Damage Type
	FGameplayTag Damage_Physical;
	FGameplayTag Damage_Magical;
	FGameplayTag Damage_Fire;
	FGameplayTag Damage_Lightning;
	FGameplayTag Damage_Ice;

	// Resistance Attributes
	FGameplayTag Attributes_Resistance_Physical;
	FGameplayTag Attributes_Resistance_Magical;
	FGameplayTag Attributes_Resistance_Fire;
	FGameplayTag Attributes_Resistance_Lightning;
	FGameplayTag Attributes_Resistance_Ice;

	// Debuff
	FGameplayTag Debuff_Burn;
	FGameplayTag Debuff_Stun;
	FGameplayTag Debuff_Ice;
	FGameplayTag Debuff_Physical;

	// Debuff Data
	FGameplayTag Debuff_Chance;
	FGameplayTag Debuff_Damage;
	FGameplayTag Debuff_Duration;
	FGameplayTag Debuff_Frequency;

	FGameplayTag Abilities_None;

	// DamageType - ResistanceToDamage를 Mapping
	TMap<FGameplayTag, FGameplayTag> DamageTypesToResistances;

	// Ability
	FGameplayTag Abilities_Attack;
	FGameplayTag Abilities_RangedAttack;

	FGameplayTag Abilities_Physical_ArrowAttack;
	FGameplayTag Abilities_Physical_ArrowRain;
	FGameplayTag Abilities_Magical_ChargingShot;
	FGameplayTag Abilities_Magical_ArrowStorm;
	FGameplayTag Abilities_Magical_ElectricShot;

	// Passive Abilities
	FGameplayTag Abilities_Passive_HaloOfProtection;
	FGameplayTag Abilities_Passive_LifeSiphon;
	FGameplayTag Abilities_Passive_ManaSiphon;

	FGameplayTag Abilities_HitReact;

	// Ability Status
	FGameplayTag Abilities_Status_Locked;
	FGameplayTag Abilities_Status_Eligible;
	FGameplayTag Abilities_Status_Unlocked;
	FGameplayTag Abilities_Status_Equipped;

	// Ability_Type
	FGameplayTag Abilities_Type_Active;
	FGameplayTag Abilities_Type_Passive;
	FGameplayTag Abilities_Type_None;

	// Cooldown (쿨타임)
	FGameplayTag Cooldown_Physical_ArrowAttack;
	// Montage Attack Socket
	FGameplayTag CombatSocket_Weapon;
	FGameplayTag CombatSocket_RightHand;
	FGameplayTag CombatSocket_LeftHand;
	FGameplayTag CombatSocket_Projectile;

	// TagContainer에서 공격에 따른 Tag를 구분하기 위함.
	FGameplayTag Montage_Attack_1;
	FGameplayTag Montage_Attack_2;
	FGameplayTag Montage_Attack_3;
	FGameplayTag Montage_Attack_4;

	TMap<FGameplayTag, FGameplayTag> DamageTypesToDebuffs;

	FGameplayTag Effects_HitReact;

	FGameplayTag Player_Block_InputPressed;
	FGameplayTag Player_Block_InputHeld;
	FGameplayTag Player_Block_InputReleased;
	FGameplayTag Player_Block_CursorTrace;

	// Item 
	FGameplayTag Item_Armor;
	FGameplayTag Item_Armor_Armor1;
	FGameplayTag Item_Armor_Armor2;
	FGameplayTag Item_Armor_Armor3;

	FGameplayTag Item_Weapon;
	FGameplayTag Item_Weapon_Bow1;
	FGameplayTag Item_Weapon_Bow2;
	FGameplayTag Item_Weapon_Bow3;

	FGameplayTag Item_Potion;
	FGameplayTag Item_Potion_HealthPotion;
	FGameplayTag Item_Potion_ManaPotion;

	// Item Status
	FGameplayTag Item_Status_None;
	FGameplayTag Item_Status_Normal;
	FGameplayTag Item_Status_Equipped;

	// Item Row Status
	FGameplayTag Item_Row_Empty;
	FGameplayTag Item_Row_Occupied;

protected:

private:
	static FDreamGameplayTags GameplayTags;
};
