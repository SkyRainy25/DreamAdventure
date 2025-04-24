// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamGameplayTags.h"
#include "GameplayTagsManager.h"

// static Get 함수의 유일한 인스턴스를 위한 GameplayTag의 타입 정의.
FDreamGameplayTags FDreamGameplayTags::GameplayTags;

void FDreamGameplayTags::InitializeNativeGameplayTags()
{
	// GameplayTag 를 언리얼에 등록.

	/*
	 * Primary Attributes
	 */
	GameplayTags.Attributes_Primary_Strength = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Strength"),
		FString("Increases physical damage")
	);

	GameplayTags.Attributes_Primary_Intelligence = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Intelligence"),
		FString("Increases magical damage")
	);

	GameplayTags.Attributes_Primary_Resilience = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Resilience"),
		FString("Increases Armor and Armor Penetration")
	);

	GameplayTags.Attributes_Primary_Vigor = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Vigor"),
		FString("Increases Health")
	);

	/*
	 * Secondary Attributes
	 */

	GameplayTags.Attributes_Secondary_Armor = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.Armor"),
		FString("Reduces damage taken, improves Block Chance")
	);

	GameplayTags.Attributes_Secondary_ArmorPenetration = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.ArmorPenetration"),
		FString("Ignores Percentage of enemy Armor, increases Critical Hit Chance")
	);

	GameplayTags.Attributes_Secondary_CriticalHitChance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.CriticalHitChance"),
		FString("Chance to double damage plus critical hit bonus")
	);

	GameplayTags.Attributes_Secondary_CriticalHitDamage = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.CriticalHitDamage"),
		FString("Bonus damage added when a critical hit is scored")
	);

	GameplayTags.Attributes_Secondary_HealthRegeneration = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.HealthRegeneration"),
		FString("Amount of Health regenerated every 1 second")
	);

	GameplayTags.Attributes_Secondary_ManaRegeneration = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.ManaRegeneration"),
		FString("Amount of Mana regenerated every 1 second")
	);

	GameplayTags.Attributes_Secondary_MaxHealth = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.MaxHealth"),
		FString("Maximum amount of Health obtainable")
	);

	GameplayTags.Attributes_Secondary_MaxMana = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.MaxMana"),
		FString("Maximum amount of Mana obtainable")
	);

	/*
	* Input Tags
	*/

	GameplayTags.InputTag_LMB = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.LMB"),
		FString("Input Tag for Left Mouse Button")
	);

	GameplayTags.InputTag_RMB = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.RMB"),
		FString("Input Tag for Right Mouse Button")
	);

	GameplayTags.InputTag_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.1"),
		FString("Input Tag for 1 key")
	);

	GameplayTags.InputTag_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.2"),
		FString("Input Tag for 2 key")
	);

	GameplayTags.InputTag_3 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.3"),
		FString("Input Tag for 3 key")
	);

	GameplayTags.InputTag_4 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.4"),
		FString("Input Tag for 4 key")
	);

	GameplayTags.InputTag_5 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.5"),
		FString("Input Tag for 5 key")
	);

	GameplayTags.InputTag_6 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.6"),
		FString("Input Tag for 6 key")
	);

	GameplayTags.InputTag_G = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.G"),
		FString("Input Tag for G key")
	);

	GameplayTags.InputTag_Passive_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.Passive.1"),
		FString("Input Tag Passive Ability 1")
	);

	GameplayTags.InputTag_Passive_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.Passive.2"),
		FString("Input Tag Passive Ability 2")
	);
	// GameplayAbility에 사용하는 Tag
	GameplayTags.Damage = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage"),
		FString("Damage")
	);

	// DamageType에 사용하는 Tag
	GameplayTags.Damage_Physical = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage.Physical"),
		FString("Physical Damage Type")
	);

	GameplayTags.Damage_Magical = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage.Magical"),
		FString("Magical Damage Type")
	);

	GameplayTags.Damage_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage.Fire"),
		FString("Fire Damage Type")
	);
	
	GameplayTags.Damage_Lightning = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage.Lightning"),
		FString("Lightning Damage Type")
	);

	GameplayTags.Damage_Ice = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage.Ice"),
		FString("Ice Damage Type")
	);

	/*
	* Debuffs
	*/

	GameplayTags.Debuff_Ice = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Debuff.Ice"),
		FString("Debuff for Ice damage")
	);
	GameplayTags.Debuff_Burn = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Debuff.Burn"),
		FString("Debuff for Fire damage")
	);
	GameplayTags.Debuff_Physical = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Debuff.Physical"),
		FString("Debuff for Physical damage")
	);
	GameplayTags.Debuff_Stun = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Debuff.Stun"),
		FString("Debuff for Lightning damage")
	);

	/* Debuff Data */
	GameplayTags.Debuff_Chance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Debuff.Chance"),
		FString("Debuff Chance")
	);
	GameplayTags.Debuff_Damage = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Debuff.Damage"),
		FString("Debuff Damage")
	);
	GameplayTags.Debuff_Duration = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Debuff.Duration"),
		FString("Debuff Duration")
	);
	GameplayTags.Debuff_Frequency = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Debuff.Frequency"),
		FString("Debuff Frequency")
	);

	//
	GameplayTags.Abilities_None = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.None"),
		FString("No Ability - like the nullptr for Ability Tags")
	);

	/*
	* Resistances
	*/

	GameplayTags.Attributes_Resistance_Physical = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Resistance.Physical"),
		FString("Resistance to Physical damage")
	);
	GameplayTags.Attributes_Resistance_Magical = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Resistance.Magical"),
		FString("Resistance to Magical damage")
	);
	GameplayTags.Attributes_Resistance_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Resistance.Fire"),
		FString("Resistance to Fire damage")
	);
	GameplayTags.Attributes_Resistance_Lightning = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Resistance.Lightning"),
		FString("Resistance to Lightning damage")
	);
	GameplayTags.Attributes_Resistance_Ice = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Resistance.Ice"),
		FString("Resistance to Ice damage")
	);

	/*
	* Meta Attributes
	*/
	GameplayTags.Attributes_Meta_IncomingXP = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Meta.IncomingXP"),
		FString("Incoming XP Meta Attribute")
	);


	GameplayTags.DamageTypesToResistances.Add(GameplayTags.Damage_Physical, GameplayTags.Attributes_Resistance_Physical);
	GameplayTags.DamageTypesToResistances.Add(GameplayTags.Damage_Magical, GameplayTags.Attributes_Resistance_Magical);
	GameplayTags.DamageTypesToResistances.Add(GameplayTags.Damage_Fire, GameplayTags.Attributes_Resistance_Fire);
	GameplayTags.DamageTypesToResistances.Add(GameplayTags.Damage_Lightning, GameplayTags.Attributes_Resistance_Lightning);
	GameplayTags.DamageTypesToResistances.Add(GameplayTags.Damage_Ice, GameplayTags.Attributes_Resistance_Ice);

	/*
	* Map of Damage Types to Debuffs
	*/
	GameplayTags.DamageTypesToDebuffs.Add(GameplayTags.Damage_Physical, GameplayTags.Debuff_Stun);
	GameplayTags.DamageTypesToDebuffs.Add(GameplayTags.Damage_Magical, GameplayTags.Debuff_Stun);
	GameplayTags.DamageTypesToDebuffs.Add(GameplayTags.Damage_Fire, GameplayTags.Debuff_Burn);
	GameplayTags.DamageTypesToDebuffs.Add(GameplayTags.Damage_Lightning, GameplayTags.Debuff_Stun);
	GameplayTags.DamageTypesToDebuffs.Add(GameplayTags.Damage_Ice, GameplayTags.Debuff_Ice);

	/*
	* AI Active Abilities
	*/

	GameplayTags.Abilities_Attack = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Attack"),
		FString("Attack Ability Tag")
	);

	GameplayTags.Abilities_RangedAttack = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.RangedAttack"),
		FString("Ranged Attack Ability Tag")
	);

	/*
	* Active Abilities
	*/

	GameplayTags.Abilities_Physical_ArrowAttack = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Physical.ArrowAttack"),
		FString("ArrowAttack Ability Tag")
	);

	GameplayTags.Abilities_Physical_ArrowRain = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Physical.ArrowRain"),
		FString("ArrowRainAbility Tag")
	);

	GameplayTags.Abilities_Magical_ChargingShot = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Magical.ChargingShot"),
		FString("ChargingShot Ability Tag")
	);

	GameplayTags.Abilities_Magical_ArrowStorm = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Magical.ArrowStorm"),
		FString("Arrow Storm Ability Tag")
	);

	GameplayTags.Abilities_Magical_ElectricShot = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Magical.ElectricShot"),
		FString("Electric Shot Ability Tag")
	);

	/*
	* Passive Spells
	*/

	GameplayTags.Abilities_Passive_LifeSiphon = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Passive.LifeSiphon"),
		FString("Life Siphon")
	);
	GameplayTags.Abilities_Passive_ManaSiphon = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Passive.ManaSiphon"),
		FString("Mana Siphon")
	);
	GameplayTags.Abilities_Passive_HaloOfProtection = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Passive.HaloOfProtection"),
		FString("Halo Of Protection")
	);

	GameplayTags.Abilities_HitReact = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.HitReact"),
		FString("Hit React Ability")
	);

	GameplayTags.Abilities_Status_Eligible = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Status.Eligible"),
		FString("Eligible Status")
	);

	GameplayTags.Abilities_Status_Equipped = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Status.Equipped"),
		FString("Equipped Status")
	);

	GameplayTags.Abilities_Status_Locked = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Status.Locked"),
		FString("Locked Status")
	);

	GameplayTags.Abilities_Status_Unlocked = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Status.Unlocked"),
		FString("Unlocked Status")
	);

	GameplayTags.Abilities_Type_None = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Type.None"),
		FString("Type None")
	);

	GameplayTags.Abilities_Type_Active = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Type.Active"),
		FString("Type Active")
	);

	GameplayTags.Abilities_Type_Passive = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Type.Passive"),
		FString("Type Passive")
	);
	/*
	* Cooldown
	*/

	GameplayTags.Cooldown_Physical_ArrowAttack = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Cooldown.Physical.ArrowAttack"),
		FString("ArrowAttack Cooldown Tag")
	);

	/*
	* Montage
	*/

	GameplayTags.CombatSocket_Weapon = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("CombatSocket.Weapon"),
		FString("Weapon")
	);

	GameplayTags.CombatSocket_RightHand = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("CombatSocket.RightHand"),
		FString("Right Hand")
	);

	GameplayTags.CombatSocket_LeftHand = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("CombatSocket.LeftHand"),
		FString("Left Hand")
	);

	GameplayTags.CombatSocket_Projectile = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("CombatSocket.Projectile"),
		FString("Projectile Socket")
	);
	/*
	* Montage Tags
	*/

	GameplayTags.Montage_Attack_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Montage.Attack.1"),
		FString("Attack 1")
	);

	GameplayTags.Montage_Attack_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Montage.Attack.2"),
		FString("Attack 2")
	);

	GameplayTags.Montage_Attack_3 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Montage.Attack.3"),
		FString("Attack 3")
	);

	GameplayTags.Montage_Attack_4 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Montage.Attack.4"),
		FString("Attack 4")
	);

	/* HitReact*/
	GameplayTags.Effects_HitReact = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Effects.HitReact"),
		FString("Tag granted when Hit Reacting")
	);

	/*
	 * Player Tags
	*/

	GameplayTags.Player_Block_CursorTrace = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Player.Block.CursorTrace"),
		FString("Block tracing under the cursor")
	);

	GameplayTags.Player_Block_InputHeld = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Player.Block.InputHeld"),
		FString("Block Input Held callback for input")
	);

	GameplayTags.Player_Block_InputPressed = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Player.Block.InputPressed"),
		FString("Block Input Pressed callback for input")
	);

	GameplayTags.Player_Block_InputReleased = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Player.Block.InputReleased"),
		FString("Block Input Released callback for input")
	);

	/*
	Item GameplayTag
	*/

	// Armor
	GameplayTags.Item_Armor = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Item.Armor"),
		FString("Item Armor")
	);
	GameplayTags.Item_Armor_Armor1 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Item.Armor.Armor1"),
		FString("Armor 1")
	);
	GameplayTags.Item_Armor_Armor2 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Item.Armor.Armor2"),
		FString("Armor 2")
	);
	GameplayTags.Item_Armor_Armor1 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Item.Armor.Armor3"),
		FString("Armor 3")
	);

	// Weapon
	GameplayTags.Item_Weapon = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Item.Weapon"),
		FString("Item Weapon")
	);
	GameplayTags.Item_Weapon_Bow1 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Item.Weapon.Bow1"),
		FString("Bow 1")
	);
	GameplayTags.Item_Weapon_Bow2 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Item.Weapon.Bow2"),
		FString("Bow 2")
	);
	GameplayTags.Item_Weapon_Bow1 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Item.Weapon.Bow3"),
		FString("Bow 3")
	);

	// Potion
	GameplayTags.Item_Potion = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Item.Potion"),
		FString("Item Potion")
	);
	GameplayTags.Item_Potion_HealthPotion = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Item.Potion.HealthPotion"),
		FString("Health Potion to heal HP")
	);
	GameplayTags.Item_Potion_ManaPotion = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Item.Potion.ManaPotion"),
		FString("Mana Potion to recover MP 2")
	);

	// Item Status Tag
	GameplayTags.Item_Status_None = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Item.Status.None"),
		FString("Item Status : None")
	);

	GameplayTags.Item_Status_Normal = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Item.Status.Normal"),
		FString("Item Status : UnEquipped")
	);

	GameplayTags.Item_Status_Equipped = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Item.Status.Equipped"),
		FString("Item Status Equipped")
	);

	/* Item Row Status */

	GameplayTags.Item_Row_Empty = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Item.Row.Empty"),
		FString("Item Row Status : Empty")
	);

	GameplayTags.Item_Row_Occupied = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Item.Row.Occupied"),
		FString("Item Row Status : Occupied")
	);
}
