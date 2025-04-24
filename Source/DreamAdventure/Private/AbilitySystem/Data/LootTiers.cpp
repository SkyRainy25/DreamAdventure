// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Data/LootTiers.h"

TArray<FLootItem> ULootTiers::GetLootItems()
{
	TArray<FLootItem> ReturnItems;

	// 몬스터가 죽으면 드랍할 아이템을 순회
	for (FLootItem& Item : LootItems)
	{
		// 각각의 아이템을 확률에 따라 드랍하도록
		for (int32 i = 0; i < Item.MaxNumberToSpawn; ++i)
		{
			// 확률
			if (FMath::FRandRange(1.f, 100.f) < Item.ChanceToSpawn)
			{
				// LootItem의 데이터를 채워넣음
				FLootItem NewItem;
				NewItem.LootClass = Item.LootClass;
				NewItem.bLootLevelOverride = Item.bLootLevelOverride;
				ReturnItems.Add(NewItem);	// 반환할 아이템(드랍 아이템)
			}
		}
	}

	return ReturnItems;
}
