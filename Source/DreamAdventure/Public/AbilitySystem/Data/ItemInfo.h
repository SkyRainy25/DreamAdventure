// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ItemInfo.generated.h"

//UENUM(BlueprintType)
//enum class EItemTypeClass
//{
//	Potion_Health = 1,
//	Potion_Mana = 2,
//	//
//	Weapon_1 = 101,
//	Weapon_2 = 102,
//	Weapon_3 = 102,
//	//
//	Armor_1 = 201,
//	Armor_2 = 202,
//	Armor_3 = 302,
//};

USTRUCT(BlueprintType)
struct FDreamItemInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag ItemTag = FGameplayTag();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag StatusTag = FGameplayTag();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 ItemTypeNumber = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 ItemAmount = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 ItemLevel = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText ItemName = FText();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText ItemDescription = FText();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AttributeTag = FGameplayTag();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 AttributeValue = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 ItemPrice = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 UpgradePrice = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 UpgradeValue = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UTexture2D> Icon = nullptr;

	friend bool operator==(const FDreamItemInfo& LeftItem, const FDreamItemInfo& RightItem)
	{
		// 아이템의 고유번호 비교를 통해, 같은 아이템인지 판별하는 operator
		return LeftItem.ItemTypeNumber == RightItem.ItemTypeNumber;
	}
};
/**
 * 
 */
UCLASS()
class DREAMADVENTURE_API UItemInfo : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ItemInformation")
	TArray<FDreamItemInfo> ItemInformation;

	FDreamItemInfo FindItemInfoFromTag(const FGameplayTag& ItemTag, bool bLogNotFound = false) const;

	FDreamItemInfo FindItemInfoFromTypeNumber(const int32 TypeNumber, bool bLogNotFound = false) const;

	FGameplayTag FindItemStatusFromItemInfo(const int32 TypeNumber, bool bLogNotFound = false) const;
};