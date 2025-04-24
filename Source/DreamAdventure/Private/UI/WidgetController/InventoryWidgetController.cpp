// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/InventoryWidgetController.h"

#include "AbilitySystem/DreamAbilitySystemComponent.h"
#include "AbilitySystem/DreamAttributeSet.h"
#include "AbilitySystem/Data/ItemInfo.h"
#include "Player/DreamPlayerState.h"
#include "DreamGameplayTags.h"

void UInventoryWidgetController::BroadcastInitialValues()
{
	OnGoldChanged.Broadcast(GetDreamPS()->GetGold());
	
	const FDreamGameplayTags Tags = FDreamGameplayTags::Get();

	for (const FDreamItemInfo& Info : GetDreamPS()->Inventory)
	{
		OnItemChanged.Broadcast(Info);
	}

	for (const FDreamItemInfo& Item : GetDreamPS()->Inventory)
	{
		if (Item.StatusTag == Tags.Item_Status_Equipped)
		{
			OnEquippedItemDelegate.Broadcast(Item);
			if(Item.ItemTag == Tags.Item_Potion_HealthPotion)
				OnHpPotionChangedDelegate.Broadcast(Item);
			else if(Item.ItemTag == Tags.Item_Potion_ManaPotion)
				OnMpPotionChangedDelegate.Broadcast(Item);
		}
	}


}

void UInventoryWidgetController::BindCallbacksToDependencies()
{
	GetDreamPS()->OnGoldChangedDelegate.AddUObject(this, &UInventoryWidgetController::GoldChanged);

	// DreamPlayerState에서 Broadcast한 Item을 UE의 Widget에 전달.
	GetDreamPS()->OnItemInfoDelegate.AddLambda(
		[this](const FDreamItemInfo& Item)
		{
			// ItemTag를 통해 Item관련 데이터를 찾아서 
			OnItemChanged.Broadcast(Item);
		}
	);
}

// 정렬 및 수량 조정된 아이템 배열을 반환하는 함수
TArray<FDreamItemInfo> UInventoryWidgetController::Sort()
{
	TArray<int32> ItemTypeNumber;

	// 실제 인벤토리에서 ItemTypeNumber를 가져옴
	for (FDreamItemInfo Item : GetDreamPS()->Inventory)
	{
		ItemTypeNumber.Add(Item.ItemTypeNumber);
	}
	// 아이템  고유번호를 크기 순으로 정렬(작은 값부터)
	ItemTypeNumber.Sort();
	
	TArray<FDreamItemInfo> OutItemList;

	int32 PrevNumber = 0;
	// 같은 아이템이라면 수량만 증가.
	for (int32 TypeNumber : ItemTypeNumber)
	{
		// 이전 아이템과 같은 아이템이 었다면?(Pass)
		if (PrevNumber == TypeNumber)	continue;
		// 아이템 
		TArray<FDreamItemInfo> Items = GetDreamPS()->FindItemArrayFromItemTypeNumber(TypeNumber);
		OutItemList.Append(Items);
		PrevNumber = TypeNumber;
	}

	return OutItemList;
	//for (const FDreamItemInfo Item : OutItemList)
	//	OnItemChanged.Broadcast(Item);
}

// 아이템의 수량을 변화시키는 함수(배열에 추가 X)
void UInventoryWidgetController::SetItemAmount(TArray<FDreamItemInfo>& ItemList, int32 TypeNumber, int32 amount)
{
	for (FDreamItemInfo& Item : ItemList)
	{
		if (Item.ItemTypeNumber == TypeNumber)
			Item.ItemAmount += amount;
	}
}

void UInventoryWidgetController::EquipItem(const FGameplayTag& AttributeTag, FGameplayTag& ItemTag, FGameplayTag& StatusTag, int32 Magnitude)
{
	UDreamAbilitySystemComponent* DreamASC = CastChecked<UDreamAbilitySystemComponent>(AbilitySystemComponent);
	DreamASC->EquipItem(AttributeTag, ItemTag, StatusTag, Magnitude);
}

void UInventoryWidgetController::RemoveItem(const FGameplayTag& AttributeTag, const FGameplayTag& StatusTag, int32 Magnitude)
{
	UDreamAbilitySystemComponent* DreamASC = CastChecked<UDreamAbilitySystemComponent>(AbilitySystemComponent);
	DreamASC->RemoveItem(AttributeTag, StatusTag, Magnitude);
}

FDreamItemInfo UInventoryWidgetController::ChangeItemStatus(FDreamItemInfo ItemData, const FGameplayTag& ItemTypeTag)
{
	const FDreamGameplayTags Tags = FDreamGameplayTags::Get();

	// 클릭한 아이템의 정보를 받아오고
	FDreamItemInfo OutItemInfo = ItemData;

	// case 1) 같은 아이템 클릭시 장착 해제
	if (ItemData.StatusTag == Tags.Item_Status_Equipped)
	{
		RemoveItem(ItemData.AttributeTag, ItemData.StatusTag, ItemData.AttributeValue);
		EquippedList.Remove(OutItemInfo);
		OutItemInfo.StatusTag = Tags.Item_Status_Normal;
		GetDreamPS()->SetInventoryItemStatus(ItemData.ItemTag, Tags.Item_Status_Normal, ItemData.ItemLevel);
		OnRemoveItemDelegate.Broadcast(OutItemInfo);
	}
	else // case 2) 현재 아이템이 장착중이지 않은 경우
	{
		bool FindEquippedItem = false;	// 장착중인 아이템이 아이템이 있는지 확인하기 위함.
		for (FDreamItemInfo& Item : GetDreamPS()->Inventory)
		{
			// 장착중인 아이템 탐색
			if (Item.StatusTag == Tags.Item_Status_Equipped)
			{
				if (Item.ItemTag.MatchesTag(ItemTypeTag))	// 문제 지점.
				{
					// 기존의 장착중인 아이템 능력치 해제
					RemoveItem(Item.AttributeTag, Item.StatusTag, Item.AttributeValue);
					EquippedList.Remove(Item);
					GetDreamPS()->SetInventoryItemStatus(Item.ItemTag, Tags.Item_Status_Normal, Item.ItemLevel);
					OnRemoveItemDelegate.Broadcast(Item);	// UE로 변경한 것을 알림

					// 장착하려는 아이템의 Status를 바꾸고 능력치 적용
					OutItemInfo = GetDreamPS()->SetInventoryItemStatus(OutItemInfo.ItemTag, Tags.Item_Status_Equipped, OutItemInfo.ItemLevel);
					EquipItem(OutItemInfo.AttributeTag, OutItemInfo.ItemTag, OutItemInfo.StatusTag, OutItemInfo.AttributeValue);
					OnEquippedItemDelegate.Broadcast(OutItemInfo); // UE로 변경한 것을 알림
					FindEquippedItem = true;	
				}
			}
		}
		// case 3) 이전에 착용장비 X => 장착하려는 아이템 장착.
		if (FindEquippedItem == false)
		{
			OutItemInfo = GetDreamPS()->SetInventoryItemStatus(OutItemInfo.ItemTag, Tags.Item_Status_Equipped, OutItemInfo.ItemLevel);
			EquipItem(OutItemInfo.AttributeTag, OutItemInfo.ItemTag, OutItemInfo.StatusTag, OutItemInfo.AttributeValue);
			OnEquippedItemDelegate.Broadcast(OutItemInfo);
		}
	}
	// 장착 슬롯에 알림
	OnItemNumberChangedDelegate.Broadcast(OutItemInfo);
	
	// case 4) 포션의 경우
	if (OutItemInfo.ItemTag == Tags.Item_Potion_HealthPotion)
		OnHpPotionChangedDelegate.Broadcast(OutItemInfo);
	else if (OutItemInfo.ItemTag == Tags.Item_Potion_ManaPotion)
		OnMpPotionChangedDelegate.Broadcast(OutItemInfo);

	OnItemEquippedDelegate.Broadcast(OutItemInfo);
	return OutItemInfo;

	/*
	// ItemData : 장착하려는 아이템
	for (FDreamItemInfo& Item : GetDreamPS()->Inventory)
	{
		// 1. 현재 장착하고 있는 장비를 한번 더 누른 경우 ( 장착 해제)
		if (ItemData.StatusTag == Tags.Item_Status_Equipped )
		{
			if (Item.ItemTag == ItemData.ItemTag && Item.StatusTag == ItemData.StatusTag)
			{
				RemoveItem(Item.AttributeTag, Item.StatusTag, Item.AttributeValue);
				Item.StatusTag = Tags.Item_Status_Normal;
				EquippedList.Remove(Item);
				OutItemInfo = Item;
				break;
			}
		}
		else
		{
			// 현재 누른 장비가 장착중이지 않은 경우
			// 아무 장비도 장착되지 않은 경우.
			Item.StatusTag.MatchesTag(Tags.Item_Status_Equipped);



		}
		// 1. 장착중인 아이템 탐색
		// ItemTypeTag 는 이 함수를 호출하는 WBP ItemBoxRow_Equipped이 어떤종류의 아이템을 담는지 나타냄
		if (Item.StatusTag.MatchesTag(Tags.Item_Status_Equipped))
		{
			// 2 인벤토리에 같은 종류의 아이템이 있는지 확인.
			if (Item.ItemTag.MatchesTag(ItemTypeTag))
			{
				// 장착중인 아이템이 동일한 아이템이라면?
				if (Item.ItemTag.MatchesTag(ItemData.ItemTag))
				{
					//  아이템을 장착 해제
					RemoveItem(Item.AttributeTag, Item.StatusTag, Item.AttributeValue);
					Item.StatusTag = Tags.Item_Status_Normal;
					EquippedList.Remove(Item);
					OutItemInfo = Item;
					OnRemoveItemDelegate.Broadcast(Item);
					break;
				}
				// 장착중인 아이템이 다른 아이템이라면?
				else
				{
					// 기존 장착중인 아이템 해제
					RemoveItem(Item.AttributeTag, Item.StatusTag, Item.AttributeValue);
					Item.StatusTag = Tags.Item_Status_Normal;
					EquippedList.Remove(Item);
					OnRemoveItemDelegate.Broadcast(Item);

					// 장착하려는 아이템 장착.
					// 실제 인벤토리에 접근해서 수정해야함!!!!
					OutItemInfo = GetDreamPS()->SetInventoryItemStatus(OutItemInfo.ItemTag, Tags.Item_Status_Equipped);

					EquipItem(OutItemInfo.AttributeTag, OutItemInfo.ItemTag, OutItemInfo.StatusTag, OutItemInfo.AttributeValue);
					break;
				}
			}
		}	// 장착중인 장비가 없는 경우
		else
		{
			// 장착하려는 아이템 장착.
			OutItemInfo = GetDreamPS()->SetInventoryItemStatus(OutItemInfo.ItemTag, Tags.Item_Status_Equipped);
			EquipItem(OutItemInfo.AttributeTag, OutItemInfo.ItemTag, OutItemInfo.StatusTag, OutItemInfo.AttributeValue);
			break;
		}*/

			// A : O B: X
			// A : X B : O
			// A : X B : X
			// 2. 그 아이템이 장착중인지 확인
			// 2-1) 장착중이지 않은 경우(같은 아이템인데 장착중이지 않은 경우) 
			// 현재 아무 장비도 착용하지 않은 경우.
			// Item = ItemData
			//if (Item.StatusTag == Tags.Item_Status_Normal && Item.ItemTag == ItemData.ItemTag)
			//{
			//	// Status : 장착중으로 바꾸고
			//	Item.StatusTag = Tags.Item_Status_Equipped;
			//	OutItemInfo = Item;
			//	EquippedList.Add(OutItemInfo);
			//	// 아이템 장착 후 능력치 적용
			//	EquipItem(OutItemInfo.AttributeTag, OutItemInfo.ItemTag, OutItemInfo.StatusTag, OutItemInfo.AttributeValue);
			//	OnItemEquippedDelegate.Broadcast(OutItemInfo);
			//}
			//// 2-2) 현재 아이템이 장착중이라면? 장착해제
			//else if (Item.ItemTag == ItemData.ItemTag && Item.StatusTag == Tags.Item_Status_Equipped)
			//{
			//	//  아이템을 장착 해제
			//	RemoveItem(Item.AttributeTag, Item.StatusTag, Item.AttributeValue);
			//	Item.StatusTag = Tags.Item_Status_Normal;
			//	EquippedList.Remove(Item);
			//}
			//// 2-3) 다른 아이템(Item)이 이미 장착중인 경우
			//else if (Item.ItemTag != ItemData.ItemTag && Item.StatusTag == Tags.Item_Status_Equipped)
			//{
			//	// 다른 아이템을 장착 해제
			//	RemoveItem(Item.AttributeTag, Item.StatusTag, Item.AttributeValue);
			//	Item.StatusTag = Tags.Item_Status_Normal;
			//	EquippedList.Remove(Item);
			//	
			//	// 장착하려는 아이템 장착.
			//	OutItemInfo.StatusTag = Tags.Item_Status_Equipped;
			//	EquipItem(OutItemInfo.AttributeTag, OutItemInfo.ItemTag, OutItemInfo.StatusTag, OutItemInfo.AttributeValue);
	
			//	OnItemEquippedDelegate.Broadcast(OutItemInfo);
			//}


	//OutItemInfo.ItemTag = ItemData.ItemTag;
	//OutItemInfo.ItemName = ItemData.ItemName;
	//OutItemInfo.ItemDescription = ItemData.ItemDescription;
	//OutItemInfo.ItemTypeNumber = ItemData.ItemTypeNumber;
	//OutItemInfo.ItemAmount = ItemData.ItemAmount;
	//OutItemInfo.ItemLevel = ItemData.ItemLevel;
	//OutItemInfo.ItemPrice = ItemData.ItemPrice;
	//OutItemInfo.AttributeTag = ItemData.AttributeTag;
	//// 강화 시스템을 도입할 경우 + ItemLevel
	//OutItemInfo.AttributeValue = ItemData.AttributeValue;
	//OutItemInfo.UpgradePrice = ItemData.UpgradePrice;
	//OutItemInfo.UpgradeValue = ItemData.UpgradeValue;
	//OutItemInfo.Icon = ItemData.Icon;

	//// 기존 장착중인 장비와 다른 장비를 장착하는 경우
	//// 다른 장비의 상태 = Item_Status_Normal
	//if (ItemData.StatusTag == Tags.Item_Status_Normal)
	//{
	//	OutItemInfo.StatusTag = Tags.Item_Status_Equipped;
	//	EquippedList.Add(OutItemInfo);
	//	// 아이템 장착 후 능력치 적용
	//	EquipItem(OutItemInfo.AttributeTag, OutItemInfo.ItemTag, OutItemInfo.StatusTag, OutItemInfo.AttributeValue);
	//	OnItemEquippedDelegate.Broadcast(OutItemInfo);
	//}
	//// 기존 장착중인 장비를 한번 더 '장착' 버튼을 누른 경우 => 장비 해제
	//else if (ItemData.StatusTag == Tags.Item_Status_Equipped)
	//{
	//	RemoveItem(OutItemInfo.AttributeTag, OutItemInfo.StatusTag, OutItemInfo.AttributeValue);
	//	OutItemInfo.StatusTag = Tags.Item_Status_Normal;
	//	EquippedList.Remove(OutItemInfo);
	//	OnItemEquippedDelegate.Broadcast(OutItemInfo);
	//}
	//// C++ 차원에서 장착 상태를 저장.
	//for (FDreamItemInfo& Item : GetDreamPS()->Inventory)
	//{
	//	if (Item.ItemTag == OutItemInfo.ItemTag)
	//	{
	//		Item.StatusTag = OutItemInfo.StatusTag;
	//	}
	//}

}

// WBP ItemBoxRow Equipped에 아이템 정보를 넘겨주기 위한 함수
FDreamItemInfo UInventoryWidgetController::GetEquippedItemInRow(const FGameplayTag& RowTypeTag)
{
	const FDreamGameplayTags Tags = FDreamGameplayTags::Get();

	for (const FDreamItemInfo& Item : GetDreamPS()->Inventory)
	{
		// 이지 장착중인 아이템 탐색
		if (Item.StatusTag == Tags.Item_Status_Equipped)
		{
			// WBP ItemBoxRow Equipped의 ItemTypeTag와 같은 종류의 아이템을 장착중이라면?
			if (Item.ItemTag.MatchesTag(RowTypeTag))
			{
				return Item;
			}
		}
	}
	return FDreamItemInfo();
}

FDreamItemInfo UInventoryWidgetController::TakeOffItem(FDreamItemInfo ItemData, const FGameplayTag& ItemTypeTag)
{
	// ItemTypeTag (
	const FDreamGameplayTags Tags = FDreamGameplayTags::Get();

	FDreamItemInfo OutItemInfo;

	for (FDreamItemInfo& Item : GetDreamPS()->Inventory)
	{
		if (Item.ItemTag.MatchesTag(ItemTypeTag))
		{
			// 장착해제
			RemoveItem(Item.AttributeTag, Item.StatusTag, Item.AttributeValue);
			Item.StatusTag = Tags.Item_Status_Normal;
			// 장착해제한 정보를 반영
			OutItemInfo = Item;
			// 장착리스트에서 제거
			EquippedList.Remove(OutItemInfo);

			OnRemoveItemDelegate.Broadcast(OutItemInfo);
		}
	}

	// 장착 슬롯에 알림
	OnItemNumberChangedDelegate.Broadcast(OutItemInfo);
	return OutItemInfo;

	//for (FDreamItemInfo& Item : GetDreamPS()->Inventory)
	//{
	//	if (Item.ItemTag == ItemData.ItemTag)
	//	{
	//		// 장착해제
	//		RemoveItem(Item.AttributeTag, Item.StatusTag, Item.AttributeValue);
	//		Item.StatusTag = Tags.Item_Status_Normal;
	//		// 장착해제한 정보를 반영
	//		OutItemInfo = Item;
	//		// 장착리스트에서 제거
	//		EquippedList.Remove(OutItemInfo);

	//		OnItemEquippedDelegate.Broadcast(OutItemInfo);

	//	}
	//}



	//OutItemInfo.ItemTag = ItemData.ItemTag;
	//OutItemInfo.ItemName = ItemData.ItemName;
	//OutItemInfo.ItemDescription = ItemData.ItemDescription;
	//OutItemInfo.ItemTypeNumber = ItemData.ItemTypeNumber;
	//OutItemInfo.ItemAmount = ItemData.ItemAmount;
	//OutItemInfo.ItemLevel = ItemData.ItemLevel;
	//OutItemInfo.ItemPrice = ItemData.ItemPrice;
	//OutItemInfo.AttributeTag = ItemData.AttributeTag;
	//OutItemInfo.AttributeValue = ItemData.AttributeValue;
	//OutItemInfo.Icon = ItemData.Icon;

	//RemoveItem(OutItemInfo.AttributeTag, ItemData.StatusTag, OutItemInfo.AttributeValue);

	//OutItemInfo.StatusTag = Tags.Item_Status_Normal;
	//EquippedList.Remove(OutItemInfo);

	//OnItemEquippedDelegate.Broadcast(OutItemInfo);
	//// C++ 차원에서 장착 상태를 저장.
	//for (FDreamItemInfo& Item : GetDreamPS()->Inventory)
	//{
	//	if (Item.ItemTag == OutItemInfo.ItemTag)
	//	{
	//		Item.StatusTag = OutItemInfo.StatusTag;
	//	}
	//}


}

void UInventoryWidgetController::ClearItem()
{
	UDreamAbilitySystemComponent* DreamASC = CastChecked<UDreamAbilitySystemComponent>(AbilitySystemComponent);
	//DreamASC->EquipItem(AttributeTag, Magnitude);
}

FDreamItemInfo UInventoryWidgetController::UpgradeItem(FDreamItemInfo ItemData)
{
	// ItemData = 업그레이드 할 아이템.
	const FDreamGameplayTags Tags = FDreamGameplayTags::Get();
	int32 UpgradePrice = ItemData.UpgradePrice;	// 업그레이드에 필요한 골드
	int32 HasGold = GetDreamPS()->GetGold();	// 가지고 있는 골드
	FGameplayTag UpgradeTag = ItemData.ItemTag;	// 업그레이드할 Item Tag
	
	int32 RemainGold = HasGold - UpgradePrice;

	FGameplayTag ArmorTag = Tags.Item_Armor;
	FGameplayTag WeaponTag = Tags.Item_Weapon;
	FGameplayTag PotionTag = Tags.Item_Potion;


	if (RemainGold >= 0)
	{
		// 골드를 재설정
		GetDreamPS()->SetGold(RemainGold);
		// 업그레이드 한 아이템이 장착중이었다면?

		// 개수가 2개 이상인 경우
		if (ItemData.ItemAmount > 1)
		{
			FDreamItemInfo PrevItem = ItemData;
			// 장착중이라면?
			if (ItemData.StatusTag == Tags.Item_Status_Equipped)
			{
				// 업그레이드 하기 전의 아이템 제거
				RemoveItem(ItemData.AttributeTag, ItemData.StatusTag, ItemData.AttributeValue);
				// 업그레이드를 하고
				FDreamItemInfo Item = GetDreamPS()->UpgradeItem(ItemData);	// 아이템 정보 증가.
				// 업그레이드 이전 장비 장착 해제
				GetDreamPS()->SetInventoryItemStatusFromTypeNumber(ItemData.ItemTypeNumber, Tags.Item_Status_Normal, ItemData.ItemLevel);	
				EquipItem(Item.AttributeTag, ItemData.ItemTag, Item.StatusTag, Item.AttributeValue);
				OnEquippedItemDelegate.Broadcast(Item);
				//OnItemChanged.Broadcast(Item);
				return Item;
			}
			else // 장착중이지 않은 경우
			{
				FDreamItemInfo Item = GetDreamPS()->UpgradeItem(ItemData);	// 아이템 정보 증가.
				//OnItemChanged.Broadcast(Item);
				return Item;
			}
		}
		else // 개수가 1개인 경우
		{
			if (ItemData.StatusTag == Tags.Item_Status_Equipped)
			{
				// 업그레이드 하기 전의 아이템 제거
				RemoveItem(ItemData.AttributeTag, ItemData.StatusTag, ItemData.AttributeValue);
				// 업그레이드를 하고
				FDreamItemInfo Item = GetDreamPS()->UpgradeItem(ItemData);
				EquipItem(Item.AttributeTag, ItemData.ItemTag, Item.StatusTag, Item.AttributeValue);
				OnEquippedItemDelegate.Broadcast(Item);
				//OnItemChanged.Broadcast(Item);
				return Item;
			}
			else // 업그레이드한 아이템이 장착중이지 않은 경우
			{
				FDreamItemInfo Item = GetDreamPS()->UpgradeItem(ItemData);
				//OnItemChanged.Broadcast(Item);
				return Item;
			}
		}
	}

	// 골드가 충분하지 않은 경우.
	return ItemData;
}

FDreamItemInfo UInventoryWidgetController::UseItem(FDreamItemInfo Item)
{
	int32 ItemNum = GetDreamPS()->FindItemInInventory(Item).ItemAmount;

	if (ItemNum == 0)	return FDreamItemInfo();

	// C++에서 플레이어(PS)가진 실제적인 데이터에서 수량 감소.
	FDreamItemInfo UsedItem = GetDreamPS()->UseItem(Item);

	UDreamAbilitySystemComponent* DreamASC = CastChecked<UDreamAbilitySystemComponent>(AbilitySystemComponent);
	DreamASC->UseItem(Item.AttributeTag, Item.StatusTag, Item.AttributeValue);

	return UsedItem;
}

FDreamItemInfo UInventoryWidgetController::UsePotion(const FGameplayTag& PotionTag)
{
	const FDreamGameplayTags Tags = FDreamGameplayTags::Get();

	FDreamItemInfo Potion = GetDreamPS()->FindItemFromTag(PotionTag);
	// 포션이 장착중이 아니라면?
	if (Potion.StatusTag == FDreamGameplayTags::Get().Item_Status_Normal)
		return Potion;

	if (Potion.ItemAmount == 0)	return Potion;

	// 실제 인벤토리(DreamPS)에서 아이템의 개수 변경
	FDreamItemInfo UsedItem = GetDreamPS()->UsePotion(PotionTag);
	int32 PotionMagnitude = UsedItem.AttributeValue;

	// 현재 플레이어의 Hp 및 Mp를 가져옴
	int32 CurrentHp = GetDreamAS()->GetHealth();
	int32 CurrentMp = GetDreamAS()->GetMana();

	// 체력 포션인 경우
	if (PotionTag == Tags.Item_Potion_HealthPotion)
	{
		// SetHealth 를 통해서 Hp 설정.
		GetDreamAS()->SetHealth(CurrentHp + PotionMagnitude);
		UE_LOG(LogTemp, Log, TEXT("Hp Potion Used"));

		//장착슬롯에 알림
		OnHpPotionChangedDelegate.Broadcast(UsedItem);
		return UsedItem;
	}
	// 마나포션인 경우
	else if (PotionTag == Tags.Item_Potion_ManaPotion)
	{
		GetDreamAS()->SetMana(CurrentMp + PotionMagnitude);
		UE_LOG(LogTemp, Log, TEXT("Mp Potion Used"));

		OnMpPotionChangedDelegate.Broadcast(UsedItem);
		return UsedItem;
	}
	return UsedItem;
}


FDreamItemInfo UInventoryWidgetController::FindEquippedItem(const FGameplayTag& ItemTag)
{
	for (const FDreamItemInfo& Item : EquippedList)
	{
		if (Item.ItemTag.MatchesTag(ItemTag))
		{
			return Item;
		}
	}
	return FDreamItemInfo();
}

bool UInventoryWidgetController::CanUpgrade(FDreamItemInfo Item)
{
	check(GetDreamPS());
	int32 GoldDifference = GetDreamPS()->GetGold() - Item.UpgradePrice;
	if (GoldDifference >= 0)	return true;
	else
		return false;
}

void UInventoryWidgetController::GoldChanged(int32 NewGold) const
{
	OnGoldChanged.Broadcast(NewGold);
}
