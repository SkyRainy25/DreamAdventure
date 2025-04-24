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

	NetUpdateFrequency = 100.f;	// 1/00초 간격으로 Replicate

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

	FDreamItemInfo RemoveItem;	// 아이템 개수가 0일 경우가 제거하기 위함.
	bool IsZero = false;
	// 인벤토리에서 실제로 가진 아이템에 접근하여 
	// 업그레이드 및 능력치 증가.
	for (FDreamItemInfo& Item : Inventory)
	{
		if (ItemData.ItemTag == Item.ItemTag && ItemData.ItemTypeNumber == Item.ItemTypeNumber)
		{
			// 기존의 아이템의 개수를 한개 줄이고
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
		// 강화하고 나서 개수가 0이 된 아이템을 인벤토리에서 삭제
		Inventory.RemoveSingle(RemoveItem);
	}

	bool IsSameItem = false;
	for (FDreamItemInfo& Item : Inventory)
	{
		// 강화한 아이템과 같은 종류의 아이템이 인벤토리에 있을 시, 개수 추가
		if ((ItemData.ItemTypeNumber + 1) == Item.ItemTypeNumber && ItemData.ItemTag.MatchesTag(Item.ItemTag))
		{
			Item.ItemAmount += 1;
			IsSameItem = true;
			OutItem =  Item;
		}
	}
	if (IsSameItem == false)	// 강화된 아이템을 이전에 가지고 있지 않은 경우
	{
		// 업그레이드된 아이템 정보 수정(능력치 증가/강화)
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
			// UI에 반영하기 위한 코드
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

			// UI에 반영하기 위한 코드
			SortList.RemoveSingle(Item);
			// 개수가 0보다 작을 경우 0으로 설정.
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
	// ItemTag에 맞는 Item 데이터를 찾아서 반환
	FDreamItemInfo ItemInfo = ItemList->FindItemInfoFromTag(ItemTag);
	// 같은 아이템이 존재하는지 판별하기 위한 flag
	bool IsSame = false;

	// C++ 차원에서 플레이어가 소유 아이템 정보 저장
	for (FDreamItemInfo& Item : Inventory)
	{
		if (Item.ItemTypeNumber == ItemInfo.ItemTypeNumber && Item.ItemLevel == ItemInfo.ItemLevel)
		{
			// 아이템을 추가(ItemAmount = 1) 할 경우
			Item.ItemAmount += ItemInfo.ItemAmount;
			IsSame = true;
		}
	}
	// Sort를 하고 UI에 보여주기 위한 인벤토리 리스트
	SortList.Add(ItemInfo);

	// 실제 데이터
	if(IsSame == false)
		Inventory.Add(ItemInfo);

	// 추가할 아이템을 Broadcast -> InventoryWC 에서 실행.
	// => UI로 띄워주기 위함.
	OnItemInfoDelegate.Broadcast(ItemInfo);
}

void ADreamPlayerState::RemoveItem(const FGameplayTag& ItemTag)
{
	FDreamItemInfo ItemInfo = ItemList->FindItemInfoFromTag(ItemTag);

	for (FDreamItemInfo& Item : Inventory)
	{
		if (Item.ItemTypeNumber == ItemInfo.ItemTypeNumber)
		{
			Item.ItemAmount -= ItemInfo.ItemAmount;	// 아이템 개수 -1
			// 음수로 가는 것 방지
			if (Item.ItemAmount <= 0)
			{
				Item.ItemAmount = 0;
				// 사용한 한개의 아이템을 제거.
				Inventory.RemoveSingle(ItemInfo);
			}
		}
	}
	SortList.RemoveSingle(ItemInfo);

	// 제거할 아이템을 Broadcast -> InventoryWC 에서 실행.
	// => UI로 띄워주기 위함.
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
	// StoreWC 로 Store에 설정할 아이템을 넘김(연결의 개념)
	OnStoreItemAddedDelegate.Broadcast(ItemTag, ItemAmount);
}
