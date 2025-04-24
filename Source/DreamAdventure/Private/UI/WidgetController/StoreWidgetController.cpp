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

	// StoreCharacter�� ������ �ִ� ��� ������(TMap)�� �޾Ƽ� 
	// UE�� �ѱ�.
	GetDreamPS()->OnStoreItemAddedDelegate.AddLambda(
		[this](const FGameplayTag& ItemTag, int32 ItemAmount)
		{
			// FDreamItemInfo�� �������� ��� ����(+ ��������)
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
	// ������ ������ ĭ�� ����ִ� ���.
	if (Item.ItemTag == FGameplayTag())	
		return;
	// �÷��̾ ������ �ִ� ��� ��������
	int32 PlayerGold = GetDreamPS()->GetGold();
	int32 ItemGold = Item.ItemPrice;
	int32 Remain = PlayerGold - ItemGold;

	// ��尡 ������� �ʰų�, ������ �������� ������ 0�� ���.
	if (Remain < 0 || Item.ItemAmount == 0)	return;

	// �÷��̾� ��� �缳��.
	GetDreamPS()->SetGold(Remain);

	// �÷��̾ ������ �߰�
	GetDreamPS()->AddToItem(Item.ItemTag);

	// ������ ������ ����Ʈ���� ���� ����.
	for (FDreamItemInfo& Info : StoreItemList)
	{
		if (Info.ItemTag.MatchesTag(Item.ItemTag))
		{	
			if (Info.ItemAmount == 0)	
				return;

			Info.ItemAmount -= 1;
			// ������ �������� ������ Widget�� ����.
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
