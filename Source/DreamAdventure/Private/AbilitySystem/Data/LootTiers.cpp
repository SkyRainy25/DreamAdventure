// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Data/LootTiers.h"

TArray<FLootItem> ULootTiers::GetLootItems()
{
	TArray<FLootItem> ReturnItems;

	// ���Ͱ� ������ ����� �������� ��ȸ
	for (FLootItem& Item : LootItems)
	{
		// ������ �������� Ȯ���� ���� ����ϵ���
		for (int32 i = 0; i < Item.MaxNumberToSpawn; ++i)
		{
			// Ȯ��
			if (FMath::FRandRange(1.f, 100.f) < Item.ChanceToSpawn)
			{
				// LootItem�� �����͸� ä������
				FLootItem NewItem;
				NewItem.LootClass = Item.LootClass;
				NewItem.bLootLevelOverride = Item.bLootLevelOverride;
				ReturnItems.Add(NewItem);	// ��ȯ�� ������(��� ������)
			}
		}
	}

	return ReturnItems;
}
