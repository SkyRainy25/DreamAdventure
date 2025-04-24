// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/DreamWidgetController.h"
#include "StoreWidgetController.generated.h"

class AStoreCharacter;
/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class DREAMADVENTURE_API UStoreWidgetController : public UDreamWidgetController
{
	GENERATED_BODY()
	
public:
	virtual void BroadcastInitialValues() override;

	virtual void BindCallbacksToDependencies() override;
	
	void AddItemToStore(const FGameplayTag& ItemTag, int32 ItemAmount);

	UFUNCTION(BlueprintCallable)
	TArray<FDreamItemInfo> GetStoreItemList();

	UFUNCTION(BlueprintCallable)
	void ClearStoreItemList();

	UFUNCTION(BlueprintCallable)
	void BuyStoreItem(const FDreamItemInfo& Item);

	UFUNCTION(BlueprintCallable)
	bool CanBuyItem(FDreamItemInfo Item);

	UPROPERTY(BlueprintAssignable, Category = "GAS|Store")
	FItemInfoDelegate OnStoreItemInfoDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Store")
	FItemInfoDelegate OnStoreItemRemoveDelegate;

	TObjectPtr<AStoreCharacter> StoreNPC;

	TArray<TObjectPtr<AStoreCharacter>> StoreArray;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Store Data")
	TArray<TSubclassOf<AStoreCharacter>> StoreList;

protected:
	TArray<FDreamItemInfo> StoreItemList;
};
