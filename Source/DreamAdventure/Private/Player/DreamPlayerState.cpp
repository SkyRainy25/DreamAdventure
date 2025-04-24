// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/DreamPlayerState.h"

#include "AbilitySystem/Data/ItemInfo.h"
#include "AbilitySystem/DreamAbilitySystemComponent.h"
#include "AbilitySystem/DreamAttributeSet.h"
#include "Net/UnrealNetwork.h"
//#include "AbilitySystem/DreamAbilitySystemLibrary.h"

ADreamPlayerState::ADreamPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UDreamAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	// GameplayEffect & GameplayCue & Gameplay Tags are Replicated
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UDreamAttributeSet>("AttributeSet");

	NetUpdateFrequency = 100.f;	// 1/00�� �������� Replicate

}

void ADreamPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADreamPlayerState, Level);
	DOREPLIFETIME(ADreamPlayerState, XP);
	DOREPLIFETIME(ADreamPlayerState, Gold);
	DOREPLIFETIME(ADreamPlayerState, AttributePoints);
	DOREPLIFETIME(ADreamPlayerState, SkillPoints);
}

UAbilitySystemComponent* ADreamPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

FDreamItemInfo ADreamPlayerState::FindItemInInventory(const FDreamItemInfo& ItemData)
{
	FDreamItemInfo OutItem = FDreamItemInfo();

	for (const FDreamItemInfo& Item : Inventory)
	{
		if (ItemData.ItemTag == Item.ItemTag)
			OutItem = Item;
	}
	return OutItem;
}

FDreamItemInfo ADreamPlayerState::FindItemFromTag(const FGameplayTag& ItemTag)
{
	FDreamItemInfo OutItem = FDreamItemInfo();

	for (const FDreamItemInfo& Item : Inventory)
	{
		if (ItemTag== Item.ItemTag)
			OutItem = Item;
	}
	return OutItem;
}

FDreamItemInfo ADreamPlayerState::UpgradeItem(FDreamItemInfo ItemData)
{
	FDreamItemInfo OutItem;

	FDreamItemInfo UpgradeItem = ItemData;

	FDreamItemInfo RemoveItem;	// ������ ������ 0�� ��찡 �����ϱ� ����.
	bool IsZero = false;
	// �κ��丮���� ������ ���� �����ۿ� �����Ͽ� 
	// ���׷��̵� �� �ɷ�ġ ����.
	for (FDreamItemInfo& Item : Inventory)
	{
		if (ItemData.ItemTag == Item.ItemTag && ItemData.ItemTypeNumber == Item.ItemTypeNumber)
		{
			// ������ �������� ������ �Ѱ� ���̰�
			Item.ItemAmount -= 1;
			if (Item.ItemAmount <= 0)
			{
				Item.ItemAmount = 0;
				RemoveItem = Item;
				IsZero = true;
			}
		}
	}
	if (IsZero)
	{
		// ��ȭ�ϰ� ���� ������ 0�� �� �������� �κ��丮���� ����
		Inventory.RemoveSingle(RemoveItem);
	}

	bool IsSameItem = false;
	for (FDreamItemInfo& Item : Inventory)
	{
		// ��ȭ�� �����۰� ���� ������ �������� �κ��丮�� ���� ��, ���� �߰�
		if ((ItemData.ItemTypeNumber + 1) == Item.ItemTypeNumber && ItemData.ItemTag.MatchesTag(Item.ItemTag))
		{
			Item.ItemAmount += 1;
			IsSameItem = true;
			OutItem =  Item;
		}
	}
	if (IsSameItem == false)	// ��ȭ�� �������� ������ ������ ���� ���� ���
	{
		// ���׷��̵�� ������ ���� ����(�ɷ�ġ ����/��ȭ)
		UpgradeItem.ItemLevel += 1;
		UpgradeItem.ItemTypeNumber += 1;
		UpgradeItem.ItemAmount = 1;
		UpgradeItem.AttributeValue += ItemData.AttributeValue;
		Inventory.Add(UpgradeItem);

		OutItem = UpgradeItem;
	}
	OnItemInfoDelegate.Broadcast(OutItem);
	return OutItem;
}

FDreamItemInfo ADreamPlayerState::UseItem(FDreamItemInfo ItemData)
{
	FDreamItemInfo OutItem = FDreamItemInfo();

	for (FDreamItemInfo& Item : Inventory)
	{
		if (ItemData.ItemTag == Item.ItemTag)
		{
			if (Item.ItemAmount == 0)
				break;

			Item.ItemAmount -= 1;
			// UI�� �ݿ��ϱ� ���� �ڵ�
			SortList.RemoveSingle(Item);

			OutItem = Item;
		}
	}
	
	return OutItem;
	
}

FDreamItemInfo ADreamPlayerState::UsePotion(const FGameplayTag& PotionTag)
{
	FDreamItemInfo OutItem = FDreamItemInfo();

	for (FDreamItemInfo& Item : Inventory)
	{
		if (PotionTag == Item.ItemTag)
		{
			Item.ItemAmount -= 1;

			// UI�� �ݿ��ϱ� ���� �ڵ�
			SortList.RemoveSingle(Item);
			// ������ 0���� ���� ��� 0���� ����.
			if (Item.ItemAmount <= 0)
			{
				Item.ItemAmount = 0; 
			}

			OutItem = Item;
		}
	}
	return OutItem;
}

void ADreamPlayerState::AddToXP(int32 InXP)
{
	XP += InXP;
	OnXPChangedDelegate.Broadcast(XP);
}

void ADreamPlayerState::AddToGold(int32 InGold)
{
	Gold += InGold;
	OnGoldChangedDelegate.Broadcast(Gold);
}

void ADreamPlayerState::AddToLevel(int32 InLevel)
{
	Level += InLevel;
	OnLevelChangedDelegate.Broadcast(Level, true);
}

void ADreamPlayerState::AddToAttributePoints(int32 InPoints)
{
	AttributePoints += InPoints;
	OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
}

void ADreamPlayerState::AddToSkillPoints(int32 InPoints)
{
	SkillPoints += InPoints;
	OnSkillPointsChangedDelegate.Broadcast(SkillPoints);
}

void ADreamPlayerState::AddToItem(const FGameplayTag& ItemTag)
{
	// ItemTag�� �´� Item �����͸� ã�Ƽ� ��ȯ
	FDreamItemInfo ItemInfo = ItemList->FindItemInfoFromTag(ItemTag);
	// ���� �������� �����ϴ��� �Ǻ��ϱ� ���� flag
	bool IsSame = false;

	// C++ �������� �÷��̾ ���� ������ ���� ����
	for (FDreamItemInfo& Item : Inventory)
	{
		if (Item.ItemTypeNumber == ItemInfo.ItemTypeNumber && Item.ItemLevel == ItemInfo.ItemLevel)
		{
			// �������� �߰�(ItemAmount = 1) �� ���
			Item.ItemAmount += ItemInfo.ItemAmount;
			IsSame = true;
		}
	}
	// Sort�� �ϰ� UI�� �����ֱ� ���� �κ��丮 ����Ʈ
	SortList.Add(ItemInfo);

	// ���� ������
	if(IsSame == false)
		Inventory.Add(ItemInfo);

	// �߰��� �������� Broadcast -> InventoryWC ���� ����.
	// => UI�� ����ֱ� ����.
	OnItemInfoDelegate.Broadcast(ItemInfo);
}

void ADreamPlayerState::RemoveItem(const FGameplayTag& ItemTag)
{
	FDreamItemInfo ItemInfo = ItemList->FindItemInfoFromTag(ItemTag);

	for (FDreamItemInfo& Item : Inventory)
	{
		if (Item.ItemTypeNumber == ItemInfo.ItemTypeNumber)
		{
			Item.ItemAmount -= ItemInfo.ItemAmount;	// ������ ���� -1
			// ������ ���� �� ����
			if (Item.ItemAmount <= 0)
			{
				Item.ItemAmount = 0;
				// ����� �Ѱ��� �������� ����.
				Inventory.RemoveSingle(ItemInfo);
			}
		}
	}
	SortList.RemoveSingle(ItemInfo);

	// ������ �������� Broadcast -> InventoryWC ���� ����.
	// => UI�� ����ֱ� ����.
	OnItemInfoDelegate.Broadcast(ItemInfo);
}

FDreamItemInfo ADreamPlayerState::FindItemFromItemTypeNumber(const int32 TypeNumber)
{
	for (const FDreamItemInfo& Item : Inventory)
	{
		if (Item.ItemTypeNumber == TypeNumber)
		{
			return Item;
		}
	}
	return FDreamItemInfo();
}

TArray<FDreamItemInfo> ADreamPlayerState::FindItemArrayFromItemTypeNumber(const int32 TypeNumber)
{
	TArray<FDreamItemInfo> OutItems;
	for (const FDreamItemInfo& Item : Inventory)
	{
		if (Item.ItemTypeNumber == TypeNumber)
		{
			OutItems.Add(Item);
		}
	}
	return OutItems;
}

FDreamItemInfo ADreamPlayerState::SetInventoryItemStatus(const FGameplayTag& ItemTag, const FGameplayTag& StatusTag, const int32& ItemLevel)
{
	for (FDreamItemInfo& Item : Inventory)
	{
		if (Item.ItemTag == ItemTag && Item.ItemLevel == ItemLevel)
		{
			Item.StatusTag = StatusTag;
			return Item;
		}
	}
	return FDreamItemInfo();
}

FDreamItemInfo ADreamPlayerState::SetInventoryItemStatusFromTypeNumber(const int32& TypeNumber, const FGameplayTag& StatusTag, const int32& ItemLevel)
{
	for (FDreamItemInfo& Item : Inventory)
	{
		if (Item.ItemTypeNumber == TypeNumber)
		{
			Item.StatusTag = StatusTag;
			return Item;
		}
	}
	return FDreamItemInfo();
}

void ADreamPlayerState::SetXP(int32 InXP)
{
	XP = InXP;
	OnXPChangedDelegate.Broadcast(XP);
}

void ADreamPlayerState::SetGold(int32 InGold)
{
	Gold = InGold;
	OnGoldChangedDelegate.Broadcast(Gold);
}

void ADreamPlayerState::SetLevel(int32 InLevel)
{
	Level = InLevel;
	OnLevelChangedDelegate.Broadcast(Level, false);
}

void ADreamPlayerState::SetAttributePoints(int32 InPoints)
{
	AttributePoints = InPoints;
	OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
}

void ADreamPlayerState::SetSkillPoints(int32 InPoints)
{
	SkillPoints = InPoints;
	OnSkillPointsChangedDelegate.Broadcast(SkillPoints);
}

void ADreamPlayerState::SetInventory(TArray<FDreamItemInfo> InInventory)
{
	Inventory = InInventory;

	for (FDreamItemInfo& Item : Inventory)
	{
		OnItemInfoDelegate.Broadcast(Item);
	}
}

void ADreamPlayerState::OnRep_Level(int32 OldLevel)
{
	OnLevelChangedDelegate.Broadcast(Level, true);
}

void ADreamPlayerState::OnRep_XP(int32 OldXP)
{
	OnXPChangedDelegate.Broadcast(XP);
}

void ADreamPlayerState::OnRep_Gold(int32 OldGold)
{
	OnGoldChangedDelegate.Broadcast(Gold);
}

void ADreamPlayerState::OnRep_AttributePoints(int32 OldAttributePoints)
{
	OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
}

void ADreamPlayerState::OnRep_SkillPoints(int32 OldSkillPoints)
{
	OnSkillPointsChangedDelegate.Broadcast(SkillPoints);
}

void ADreamPlayerState::AddItemToStoreMenu(const FGameplayTag& ItemTag, int32 ItemAmount)
{
	// StoreWC �� Store�� ������ �������� �ѱ�(������ ����)
	OnStoreItemAddedDelegate.Broadcast(ItemTag, ItemAmount);
}
