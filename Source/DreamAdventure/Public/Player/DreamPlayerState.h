// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerState.h"
#include "DreamPlayerState.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class ULevelupInfo;
class UItemInfo;
struct FDreamItemInfo;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChanged, int32 /*StatValue*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemInfoChanged, const FDreamItemInfo& /*ItemInfo*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemAmountChanged, const FDreamItemInfo& /*ItemInfo*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnStoreItemAdded, const FGameplayTag&, int32);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLevelChanged, int32 /*StatValue*/, bool /*bLevelUp*/)
/**
 * 
 */
UCLASS()
class DREAMADVENTURE_API ADreamPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	ADreamPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }
	
	FOnPlayerStatChanged OnXPChangedDelegate;
	FOnPlayerStatChanged OnGoldChangedDelegate;
	FOnPlayerStatChanged OnAttributePointsChangedDelegate;
	FOnPlayerStatChanged OnSkillPointsChangedDelegate;

	FOnLevelChanged OnLevelChangedDelegate;

	FOnItemInfoChanged OnItemInfoDelegate;
	FOnItemAmountChanged OnItemAmountDelegate;

	FOnStoreItemAdded OnStoreItemAddedDelegate;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<ULevelupInfo> LevelUpInfo;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ItemList")
	TObjectPtr<UItemInfo> ItemList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ItemList")
	TArray<FDreamItemInfo> Inventory;

	// Sort한 이후 UI로 보여주기 위한 리스트
	TArray<FDreamItemInfo> SortList;

	/* Item에 관한 함수 (업그레이드 및 아이템 사용) */
	FDreamItemInfo FindItemInInventory(const FDreamItemInfo& ItemData);
	FDreamItemInfo FindItemFromTag(const FGameplayTag& ItemTag);
	FDreamItemInfo FindItemFromItemTypeNumber(const int32 TypeNumber);
	TArray<FDreamItemInfo> FindItemArrayFromItemTypeNumber(const int32 TypeNumber);
	FDreamItemInfo SetInventoryItemStatus(const FGameplayTag& ItemTag, const FGameplayTag& StatusTag, const int32& ItemLevel);
	FDreamItemInfo SetInventoryItemStatusFromTypeNumber(const int32& TypeNumber, const FGameplayTag& StatusTag, const int32& ItemLevel);
	FDreamItemInfo UpgradeItem(FDreamItemInfo ItemData);

	FDreamItemInfo UseItem(FDreamItemInfo ItemData);
	FDreamItemInfo UsePotion(const FGameplayTag& PotionTag);

;	// 작은 최적화를 위한 매크로
	FORCEINLINE int32 GetPlayerLevel() const { return Level; }
	FORCEINLINE int32 GetXP() const { return XP; }
	FORCEINLINE int32 GetGold() const { return Gold; }
	FORCEINLINE int32 GetAttributePoints() const { return AttributePoints; }
	FORCEINLINE int32 GetSkillPoints() const { return SkillPoints; }
	FORCEINLINE TArray<FDreamItemInfo> GetInventory() const { return Inventory; }

	void AddToXP(int32 InXP);
	void AddToGold(int32 InGold);
	void AddToLevel(int32 InLevel);
	void AddToAttributePoints(int32 InPoints);
	void AddToSkillPoints(int32 InPoints);
	void AddToItem(const FGameplayTag& ItemTag);
	void RemoveItem(const FGameplayTag& ItemTag);
	
	void AddItemToStoreMenu(const FGameplayTag& ItemTag, int32 ItemAmount);

	void SetXP(int32 InXP);
	void SetGold(int32 InGold);
	void SetLevel(int32 InLevel);
	void SetAttributePoints(int32 InPoints);
	void SetSkillPoints(int32 InPoints);
	void SetInventory(TArray<FDreamItemInfo> InInventory);
protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

private:

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Level)
	int32 Level = 1;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_XP)
	int32 XP = 0;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Gold)
	int32 Gold = 1000;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_AttributePoints)
	int32 AttributePoints = 0;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_SkillPoints)
	int32 SkillPoints = 0;

	UFUNCTION()
	void OnRep_Level(int32 OldLevel);

	UFUNCTION()
	void OnRep_XP(int32 OldXP);

	UFUNCTION()
	void OnRep_Gold(int32 OldGold);

	UFUNCTION()
	void OnRep_AttributePoints(int32 OldAttributePoints);

	UFUNCTION()
	void OnRep_SkillPoints(int32 OldSkillPoints);
};
