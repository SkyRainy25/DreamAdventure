// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Data/ItemInfo.h"

#include "AbilitySystem/DreamAbilitySystemLibrary.h"

FDreamItemInfo UItemInfo::FindItemInfoFromTag(const FGameplayTag& ItemTag, bool bLogNotFound) const
{
	for (const FDreamItemInfo Info : ItemInformation)
	{
		// 아이템 Tag 에 맞는 아이템을 찾아 return
		if (Info.ItemTag == ItemTag)
		{
			return Info;
		}
	}
	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find info for ItemTag [%s] on ItemInfo [%s]"), *ItemTag.ToString(), *GetNameSafe(this));
	}
	
	return FDreamItemInfo();
}

FDreamItemInfo UItemInfo::FindItemInfoFromTypeNumber(const int32 TypeNumber, bool bLogNotFound) const
{
	for (const FDreamItemInfo Info : ItemInformation)
	{
		if (Info.ItemTypeNumber == TypeNumber)
		{
			return Info;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("ItemTypeNumber is not found"));
	}

	return FDreamItemInfo();
}

FGameplayTag UItemInfo::FindItemStatusFromItemInfo(const int32 TypeNumber, bool bLogNotFound) const
{
	return FGameplayTag();
}
