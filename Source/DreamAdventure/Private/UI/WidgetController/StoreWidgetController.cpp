// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/StoreWidgetController.h"
#include "DreamGameplayTags.h"
#include "AbilitySystem/Data/StoreClass.h"
#include "AbilitySystem/Data/ItemInfo.h"
#include "Character/Store/StoreCharacter.h"
#include "Player/DreamPlayerState.h"

void UStoreWidgetController::BroadcastInitialValues()
{
	const FDreamGameplayTags Tags = FDreamGameplayTags::Get();
}

void UStoreWidgetController::BindCallbacksToDependencies()
{
	const FDreamGameplayTags Tags = FDreamGameplayTags::Get();

	// StoreCharacter이 가지고 있는 모든 아이템(TMap)을 받아서 
	// UE로 넘김.
	GetDreamPS()->OnStoreItemAddedDelegate.AddLambda(
		[this](const FGameplayTag& ItemTag, int32 ItemAmount)
		{
			// FDreamItemInfo의 형식으로 묶어서 보냄(+ 수량조절)
			FDreamItemInfo Item = ItemInfo->FindItemInfoFromTag(ItemTag);
			Item.ItemAmount = ItemAmount;
			StoreItemList.Add(Item);
			OnStoreItemInfoDelegate.Broadcast(Item);
		}
	);
}

void UStoreWidgetController::AddItemToStore(const FGameplayTag& ItemTag, int32 ItemAmount)
{

}

TArray<FDreamItemInfo> UStoreWidgetController::GetStoreItemList()
{
	return StoreItemList;
}

void UStoreWidgetController::ClearStoreItemList()
{
	if (StoreList.Num() == 0)	return;
	
	StoreItemList.Empty();
}

void UStoreWidgetController::BuyStoreItem(const FDreamItemInfo& Item)
{
	// 상점의 아이템 칸이 비어있는 경우.
	if (Item.ItemTag == FGameplayTag())	
		return;
	// 플레이어가 가지고 있는 골드 가져오고
	int32 PlayerGold = GetDreamPS()->GetGold();
	int32 ItemGold = Item.ItemPrice;
	int32 Remain = PlayerGold - ItemGold;

	// 골드가 충분하지 않거나, 상점의 아이템이 개수가 0이 경우.
	if (Remain < 0 || Item.ItemAmount == 0)	return;

	// 플레이어 골드 재설정.
	GetDreamPS()->SetGold(Remain);

	// 플레이어에 아이템 추가
	GetDreamPS()->AddToItem(Item.ItemTag);

	// 상점의 아이템 리스트에서 개수 감소.
	for (FDreamItemInfo& Info : StoreItemList)
	{
		if (Info.ItemTag.MatchesTag(Item.ItemTag))
		{	
			if (Info.ItemAmount == 0)	
				return;

			Info.ItemAmount -= 1;
			// 수정된 아이템의 정보를 Widget에 전달.
			OnStoreItemInfoDelegate.Broadcast(Info);
		}
	}
}

bool UStoreWidgetController::CanBuyItem(FDreamItemInfo Item)
{
	check(GetDreamPS());
	int32 GoldDifference = GetDreamPS()->GetGold() - Item.ItemPrice;
	if (GoldDifference >= 0)	return true;
	else
		return false;
}
