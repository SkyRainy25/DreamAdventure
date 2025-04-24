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

	// DreamPlayerState���� Broadcast�� Item�� UE�� Widget�� ����.
	GetDreamPS()->OnItemInfoDelegate.AddLambda(
		[this](const FDreamItemInfo& Item)
		{
			// ItemTag�� ���� Item���� �����͸� ã�Ƽ� 
			OnItemChanged.Broadcast(Item);
		}
	);
}

// ���� �� ���� ������ ������ �迭�� ��ȯ�ϴ� �Լ�
TArray<FDreamItemInfo> UInventoryWidgetController::Sort()
{
	TArray<int32> ItemTypeNumber;

	// ���� �κ��丮���� ItemTypeNumber�� ������
	for (FDreamItemInfo Item : GetDreamPS()->Inventory)
	{
		ItemTypeNumber.Add(Item.ItemTypeNumber);
	}
	// ������  ������ȣ�� ũ�� ������ ����(���� ������)
	ItemTypeNumber.Sort();
	
	TArray<FDreamItemInfo> OutItemList;

	int32 PrevNumber = 0;
	// ���� �������̶�� ������ ����.
	for (int32 TypeNumber : ItemTypeNumber)
	{
		// ���� �����۰� ���� �������� ���ٸ�?(Pass)
		if (PrevNumber == TypeNumber)	continue;
		// ������ 
		TArray<FDreamItemInfo> Items = GetDreamPS()->FindItemArrayFromItemTypeNumber(TypeNumber);
		OutItemList.Append(Items);
		PrevNumber = TypeNumber;
	}

	return OutItemList;
	//for (const FDreamItemInfo Item : OutItemList)
	//	OnItemChanged.Broadcast(Item);
}

// �������� ������ ��ȭ��Ű�� �Լ�(�迭�� �߰� X)
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

	// Ŭ���� �������� ������ �޾ƿ���
	FDreamItemInfo OutItemInfo = ItemData;

	// case 1) ���� ������ Ŭ���� ���� ����
	if (ItemData.StatusTag == Tags.Item_Status_Equipped)
	{
		RemoveItem(ItemData.AttributeTag, ItemData.StatusTag, ItemData.AttributeValue);
		EquippedList.Remove(OutItemInfo);
		OutItemInfo.StatusTag = Tags.Item_Status_Normal;
		GetDreamPS()->SetInventoryItemStatus(ItemData.ItemTag, Tags.Item_Status_Normal, ItemData.ItemLevel);
		OnRemoveItemDelegate.Broadcast(OutItemInfo);
	}
	else // case 2) ���� �������� ���������� ���� ���
	{
		bool FindEquippedItem = false;	// �������� �������� �������� �ִ��� Ȯ���ϱ� ����.
		for (FDreamItemInfo& Item : GetDreamPS()->Inventory)
		{
			// �������� ������ Ž��
			if (Item.StatusTag == Tags.Item_Status_Equipped)
			{
				if (Item.ItemTag.MatchesTag(ItemTypeTag))	// ���� ����.
				{
					// ������ �������� ������ �ɷ�ġ ����
					RemoveItem(Item.AttributeTag, Item.StatusTag, Item.AttributeValue);
					EquippedList.Remove(Item);
					GetDreamPS()->SetInventoryItemStatus(Item.ItemTag, Tags.Item_Status_Normal, Item.ItemLevel);
					OnRemoveItemDelegate.Broadcast(Item);	// UE�� ������ ���� �˸�

					// �����Ϸ��� �������� Status�� �ٲٰ� �ɷ�ġ ����
					OutItemInfo = GetDreamPS()->SetInventoryItemStatus(OutItemInfo.ItemTag, Tags.Item_Status_Equipped, OutItemInfo.ItemLevel);
					EquipItem(OutItemInfo.AttributeTag, OutItemInfo.ItemTag, OutItemInfo.StatusTag, OutItemInfo.AttributeValue);
					OnEquippedItemDelegate.Broadcast(OutItemInfo); // UE�� ������ ���� �˸�
					FindEquippedItem = true;	
				}
			}
		}
		// case 3) ������ ������� X => �����Ϸ��� ������ ����.
		if (FindEquippedItem == false)
		{
			OutItemInfo = GetDreamPS()->SetInventoryItemStatus(OutItemInfo.ItemTag, Tags.Item_Status_Equipped, OutItemInfo.ItemLevel);
			EquipItem(OutItemInfo.AttributeTag, OutItemInfo.ItemTag, OutItemInfo.StatusTag, OutItemInfo.AttributeValue);
			OnEquippedItemDelegate.Broadcast(OutItemInfo);
		}
	}
	// ���� ���Կ� �˸�
	OnItemNumberChangedDelegate.Broadcast(OutItemInfo);
	
	// case 4) ������ ���
	if (OutItemInfo.ItemTag == Tags.Item_Potion_HealthPotion)
		OnHpPotionChangedDelegate.Broadcast(OutItemInfo);
	else if (OutItemInfo.ItemTag == Tags.Item_Potion_ManaPotion)
		OnMpPotionChangedDelegate.Broadcast(OutItemInfo);

	OnItemEquippedDelegate.Broadcast(OutItemInfo);
	return OutItemInfo;

	/*
	// ItemData : �����Ϸ��� ������
	for (FDreamItemInfo& Item : GetDreamPS()->Inventory)
	{
		// 1. ���� �����ϰ� �ִ� ��� �ѹ� �� ���� ��� ( ���� ����)
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
			// ���� ���� ��� ���������� ���� ���
			// �ƹ� ��� �������� ���� ���.
			Item.StatusTag.MatchesTag(Tags.Item_Status_Equipped);



		}
		// 1. �������� ������ Ž��
		// ItemTypeTag �� �� �Լ��� ȣ���ϴ� WBP ItemBoxRow_Equipped�� ������� �������� ����� ��Ÿ��
		if (Item.StatusTag.MatchesTag(Tags.Item_Status_Equipped))
		{
			// 2 �κ��丮�� ���� ������ �������� �ִ��� Ȯ��.
			if (Item.ItemTag.MatchesTag(ItemTypeTag))
			{
				// �������� �������� ������ �������̶��?
				if (Item.ItemTag.MatchesTag(ItemData.ItemTag))
				{
					//  �������� ���� ����
					RemoveItem(Item.AttributeTag, Item.StatusTag, Item.AttributeValue);
					Item.StatusTag = Tags.Item_Status_Normal;
					EquippedList.Remove(Item);
					OutItemInfo = Item;
					OnRemoveItemDelegate.Broadcast(Item);
					break;
				}
				// �������� �������� �ٸ� �������̶��?
				else
				{
					// ���� �������� ������ ����
					RemoveItem(Item.AttributeTag, Item.StatusTag, Item.AttributeValue);
					Item.StatusTag = Tags.Item_Status_Normal;
					EquippedList.Remove(Item);
					OnRemoveItemDelegate.Broadcast(Item);

					// �����Ϸ��� ������ ����.
					// ���� �κ��丮�� �����ؼ� �����ؾ���!!!!
					OutItemInfo = GetDreamPS()->SetInventoryItemStatus(OutItemInfo.ItemTag, Tags.Item_Status_Equipped);

					EquipItem(OutItemInfo.AttributeTag, OutItemInfo.ItemTag, OutItemInfo.StatusTag, OutItemInfo.AttributeValue);
					break;
				}
			}
		}	// �������� ��� ���� ���
		else
		{
			// �����Ϸ��� ������ ����.
			OutItemInfo = GetDreamPS()->SetInventoryItemStatus(OutItemInfo.ItemTag, Tags.Item_Status_Equipped);
			EquipItem(OutItemInfo.AttributeTag, OutItemInfo.ItemTag, OutItemInfo.StatusTag, OutItemInfo.AttributeValue);
			break;
		}*/

			// A : O B: X
			// A : X B : O
			// A : X B : X
			// 2. �� �������� ���������� Ȯ��
			// 2-1) ���������� ���� ���(���� �������ε� ���������� ���� ���) 
			// ���� �ƹ� ��� �������� ���� ���.
			// Item = ItemData
			//if (Item.StatusTag == Tags.Item_Status_Normal && Item.ItemTag == ItemData.ItemTag)
			//{
			//	// Status : ���������� �ٲٰ�
			//	Item.StatusTag = Tags.Item_Status_Equipped;
			//	OutItemInfo = Item;
			//	EquippedList.Add(OutItemInfo);
			//	// ������ ���� �� �ɷ�ġ ����
			//	EquipItem(OutItemInfo.AttributeTag, OutItemInfo.ItemTag, OutItemInfo.StatusTag, OutItemInfo.AttributeValue);
			//	OnItemEquippedDelegate.Broadcast(OutItemInfo);
			//}
			//// 2-2) ���� �������� �������̶��? ��������
			//else if (Item.ItemTag == ItemData.ItemTag && Item.StatusTag == Tags.Item_Status_Equipped)
			//{
			//	//  �������� ���� ����
			//	RemoveItem(Item.AttributeTag, Item.StatusTag, Item.AttributeValue);
			//	Item.StatusTag = Tags.Item_Status_Normal;
			//	EquippedList.Remove(Item);
			//}
			//// 2-3) �ٸ� ������(Item)�� �̹� �������� ���
			//else if (Item.ItemTag != ItemData.ItemTag && Item.StatusTag == Tags.Item_Status_Equipped)
			//{
			//	// �ٸ� �������� ���� ����
			//	RemoveItem(Item.AttributeTag, Item.StatusTag, Item.AttributeValue);
			//	Item.StatusTag = Tags.Item_Status_Normal;
			//	EquippedList.Remove(Item);
			//	
			//	// �����Ϸ��� ������ ����.
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
	//// ��ȭ �ý����� ������ ��� + ItemLevel
	//OutItemInfo.AttributeValue = ItemData.AttributeValue;
	//OutItemInfo.UpgradePrice = ItemData.UpgradePrice;
	//OutItemInfo.UpgradeValue = ItemData.UpgradeValue;
	//OutItemInfo.Icon = ItemData.Icon;

	//// ���� �������� ���� �ٸ� ��� �����ϴ� ���
	//// �ٸ� ����� ���� = Item_Status_Normal
	//if (ItemData.StatusTag == Tags.Item_Status_Normal)
	//{
	//	OutItemInfo.StatusTag = Tags.Item_Status_Equipped;
	//	EquippedList.Add(OutItemInfo);
	//	// ������ ���� �� �ɷ�ġ ����
	//	EquipItem(OutItemInfo.AttributeTag, OutItemInfo.ItemTag, OutItemInfo.StatusTag, OutItemInfo.AttributeValue);
	//	OnItemEquippedDelegate.Broadcast(OutItemInfo);
	//}
	//// ���� �������� ��� �ѹ� �� '����' ��ư�� ���� ��� => ��� ����
	//else if (ItemData.StatusTag == Tags.Item_Status_Equipped)
	//{
	//	RemoveItem(OutItemInfo.AttributeTag, OutItemInfo.StatusTag, OutItemInfo.AttributeValue);
	//	OutItemInfo.StatusTag = Tags.Item_Status_Normal;
	//	EquippedList.Remove(OutItemInfo);
	//	OnItemEquippedDelegate.Broadcast(OutItemInfo);
	//}
	//// C++ �������� ���� ���¸� ����.
	//for (FDreamItemInfo& Item : GetDreamPS()->Inventory)
	//{
	//	if (Item.ItemTag == OutItemInfo.ItemTag)
	//	{
	//		Item.StatusTag = OutItemInfo.StatusTag;
	//	}
	//}

}

// WBP ItemBoxRow Equipped�� ������ ������ �Ѱ��ֱ� ���� �Լ�
FDreamItemInfo UInventoryWidgetController::GetEquippedItemInRow(const FGameplayTag& RowTypeTag)
{
	const FDreamGameplayTags Tags = FDreamGameplayTags::Get();

	for (const FDreamItemInfo& Item : GetDreamPS()->Inventory)
	{
		// ���� �������� ������ Ž��
		if (Item.StatusTag == Tags.Item_Status_Equipped)
		{
			// WBP ItemBoxRow Equipped�� ItemTypeTag�� ���� ������ �������� �������̶��?
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
			// ��������
			RemoveItem(Item.AttributeTag, Item.StatusTag, Item.AttributeValue);
			Item.StatusTag = Tags.Item_Status_Normal;
			// ���������� ������ �ݿ�
			OutItemInfo = Item;
			// ��������Ʈ���� ����
			EquippedList.Remove(OutItemInfo);

			OnRemoveItemDelegate.Broadcast(OutItemInfo);
		}
	}

	// ���� ���Կ� �˸�
	OnItemNumberChangedDelegate.Broadcast(OutItemInfo);
	return OutItemInfo;

	//for (FDreamItemInfo& Item : GetDreamPS()->Inventory)
	//{
	//	if (Item.ItemTag == ItemData.ItemTag)
	//	{
	//		// ��������
	//		RemoveItem(Item.AttributeTag, Item.StatusTag, Item.AttributeValue);
	//		Item.StatusTag = Tags.Item_Status_Normal;
	//		// ���������� ������ �ݿ�
	//		OutItemInfo = Item;
	//		// ��������Ʈ���� ����
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
	//// C++ �������� ���� ���¸� ����.
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
	// ItemData = ���׷��̵� �� ������.
	const FDreamGameplayTags Tags = FDreamGameplayTags::Get();
	int32 UpgradePrice = ItemData.UpgradePrice;	// ���׷��̵忡 �ʿ��� ���
	int32 HasGold = GetDreamPS()->GetGold();	// ������ �ִ� ���
	FGameplayTag UpgradeTag = ItemData.ItemTag;	// ���׷��̵��� Item Tag
	
	int32 RemainGold = HasGold - UpgradePrice;

	FGameplayTag ArmorTag = Tags.Item_Armor;
	FGameplayTag WeaponTag = Tags.Item_Weapon;
	FGameplayTag PotionTag = Tags.Item_Potion;


	if (RemainGold >= 0)
	{
		// ��带 �缳��
		GetDreamPS()->SetGold(RemainGold);
		// ���׷��̵� �� �������� �������̾��ٸ�?

		// ������ 2�� �̻��� ���
		if (ItemData.ItemAmount > 1)
		{
			FDreamItemInfo PrevItem = ItemData;
			// �������̶��?
			if (ItemData.StatusTag == Tags.Item_Status_Equipped)
			{
				// ���׷��̵� �ϱ� ���� ������ ����
				RemoveItem(ItemData.AttributeTag, ItemData.StatusTag, ItemData.AttributeValue);
				// ���׷��̵带 �ϰ�
				FDreamItemInfo Item = GetDreamPS()->UpgradeItem(ItemData);	// ������ ���� ����.
				// ���׷��̵� ���� ��� ���� ����
				GetDreamPS()->SetInventoryItemStatusFromTypeNumber(ItemData.ItemTypeNumber, Tags.Item_Status_Normal, ItemData.ItemLevel);	
				EquipItem(Item.AttributeTag, ItemData.ItemTag, Item.StatusTag, Item.AttributeValue);
				OnEquippedItemDelegate.Broadcast(Item);
				//OnItemChanged.Broadcast(Item);
				return Item;
			}
			else // ���������� ���� ���
			{
				FDreamItemInfo Item = GetDreamPS()->UpgradeItem(ItemData);	// ������ ���� ����.
				//OnItemChanged.Broadcast(Item);
				return Item;
			}
		}
		else // ������ 1���� ���
		{
			if (ItemData.StatusTag == Tags.Item_Status_Equipped)
			{
				// ���׷��̵� �ϱ� ���� ������ ����
				RemoveItem(ItemData.AttributeTag, ItemData.StatusTag, ItemData.AttributeValue);
				// ���׷��̵带 �ϰ�
				FDreamItemInfo Item = GetDreamPS()->UpgradeItem(ItemData);
				EquipItem(Item.AttributeTag, ItemData.ItemTag, Item.StatusTag, Item.AttributeValue);
				OnEquippedItemDelegate.Broadcast(Item);
				//OnItemChanged.Broadcast(Item);
				return Item;
			}
			else // ���׷��̵��� �������� ���������� ���� ���
			{
				FDreamItemInfo Item = GetDreamPS()->UpgradeItem(ItemData);
				//OnItemChanged.Broadcast(Item);
				return Item;
			}
		}
	}

	// ��尡 ������� ���� ���.
	return ItemData;
}

FDreamItemInfo UInventoryWidgetController::UseItem(FDreamItemInfo Item)
{
	int32 ItemNum = GetDreamPS()->FindItemInInventory(Item).ItemAmount;

	if (ItemNum == 0)	return FDreamItemInfo();

	// C++���� �÷��̾�(PS)���� �������� �����Ϳ��� ���� ����.
	FDreamItemInfo UsedItem = GetDreamPS()->UseItem(Item);

	UDreamAbilitySystemComponent* DreamASC = CastChecked<UDreamAbilitySystemComponent>(AbilitySystemComponent);
	DreamASC->UseItem(Item.AttributeTag, Item.StatusTag, Item.AttributeValue);

	return UsedItem;
}

FDreamItemInfo UInventoryWidgetController::UsePotion(const FGameplayTag& PotionTag)
{
	const FDreamGameplayTags Tags = FDreamGameplayTags::Get();

	FDreamItemInfo Potion = GetDreamPS()->FindItemFromTag(PotionTag);
	// ������ �������� �ƴ϶��?
	if (Potion.StatusTag == FDreamGameplayTags::Get().Item_Status_Normal)
		return Potion;

	if (Potion.ItemAmount == 0)	return Potion;

	// ���� �κ��丮(DreamPS)���� �������� ���� ����
	FDreamItemInfo UsedItem = GetDreamPS()->UsePotion(PotionTag);
	int32 PotionMagnitude = UsedItem.AttributeValue;

	// ���� �÷��̾��� Hp �� Mp�� ������
	int32 CurrentHp = GetDreamAS()->GetHealth();
	int32 CurrentMp = GetDreamAS()->GetMana();

	// ü�� ������ ���
	if (PotionTag == Tags.Item_Potion_HealthPotion)
	{
		// SetHealth �� ���ؼ� Hp ����.
		GetDreamAS()->SetHealth(CurrentHp + PotionMagnitude);
		UE_LOG(LogTemp, Log, TEXT("Hp Potion Used"));

		//�������Կ� �˸�
		OnHpPotionChangedDelegate.Broadcast(UsedItem);
		return UsedItem;
	}
	// ���������� ���
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
