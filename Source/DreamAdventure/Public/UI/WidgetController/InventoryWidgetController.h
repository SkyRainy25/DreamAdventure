// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/DreamWidgetController.h"
#include "InventoryWidgetController.generated.h"

class UItemInfo;
struct FDreamItemInfo;
/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class DREAMADVENTURE_API UInventoryWidgetController : public UDreamWidgetController
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable, Category="GAS|Gold")
	FOnPlayerStatChangedSignature OnGoldChanged;

	virtual void BroadcastInitialValues() override;

	virtual void BindCallbacksToDependencies() override;

	UFUNCTION(BlueprintCallable)
	TArray<FDreamItemInfo> Sort();

	void SetItemAmount(TArray<FDreamItemInfo>& ItemList, int32 TypeNumber, int32 amount);

	UFUNCTION(BlueprintCallable)
	void EquipItem(const FGameplayTag& AttributeTag, FGameplayTag& ItemTag, FGameplayTag& StatusTag, int32 Magnitude);

	UFUNCTION(BlueprintCallable)
	void RemoveItem(const FGameplayTag& AttributeTag, const FGameplayTag& StatusTag, int32 Magnitude);

	UFUNCTION(BlueprintCallable)
	FDreamItemInfo ChangeItemStatus(FDreamItemInfo ItemData, const FGameplayTag& ItemTypeTag);

	UFUNCTION(BlueprintCallable)
	FDreamItemInfo GetEquippedItemInRow(const FGameplayTag& RowTypeTag);

	UFUNCTION(BlueprintCallable)
	FDreamItemInfo TakeOffItem(FDreamItemInfo ItemData, const FGameplayTag& ItemTypeTag);

	UFUNCTION(BlueprintCallable)
	void ClearItem();

	UFUNCTION(BlueprintCallable)
	FDreamItemInfo UpgradeItem(FDreamItemInfo ItemData);

	UFUNCTION(BlueprintCallable)
	FDreamItemInfo UseItem(FDreamItemInfo Item);

	UFUNCTION(BlueprintCallable)
	FDreamItemInfo UsePotion(const FGameplayTag& PotionTag);


	TArray<FDreamItemInfo> EquippedList;

	UFUNCTION(BlueprintCallable)
	TArray<FDreamItemInfo> GetEquippedList() { return EquippedList; }

	UFUNCTION(BlueprintCallable)
	FDreamItemInfo FindEquippedItem(const FGameplayTag& ItemTag);

	UFUNCTION(BlueprintCallable)
	bool CanUpgrade(FDreamItemInfo Item);

	TArray<FDreamItemInfo> UpdateList;
protected:

	void GoldChanged(int32 NewGold) const;

	//void AddItem(const UItemInfo& ItemInfo);
};
