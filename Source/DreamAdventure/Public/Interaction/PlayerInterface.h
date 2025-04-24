// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/Data/ItemInfo.h"
#include "PlayerInterface.generated.h"

struct FDreamItemInfo;
struct FGaeplayTag;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DREAMADVENTURE_API IPlayerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent)
	int32 FindLevelForXP(int32 InXP) const;

	UFUNCTION(BlueprintNativeEvent)
	int32 GetXP() const;

	UFUNCTION(BlueprintNativeEvent)
	int32 GetAttributePointsReward(int32 Level) const;

	UFUNCTION(BlueprintNativeEvent)
	int32 GetSkillPointsReward(int32 Level) const;

	UFUNCTION(BlueprintNativeEvent)
	void AddToXP(int32 InXP);

	UFUNCTION(BlueprintNativeEvent)
	void AddToGold(int32 InXP);

	UFUNCTION(BlueprintNativeEvent)
	void LevelUp();

	UFUNCTION(BlueprintNativeEvent)
	void AddToPlayerLevel(int32 InPlayerLevel);

	UFUNCTION(BlueprintNativeEvent)
	void AddToAttributePoints(int32 InAttributePoints);

	UFUNCTION(BlueprintNativeEvent)
	int32 GetAttributePoints() const;

	UFUNCTION(BlueprintNativeEvent)
	void AddToSkillPoints(int32 InSkillPoints);

	UFUNCTION(BlueprintNativeEvent)
	int32 GetSkillPoints() const;

	UFUNCTION(BlueprintNativeEvent)
	FDreamItemInfo GetItem() const;

	UFUNCTION(BlueprintNativeEvent)
	int32 GetGold() const;

	UFUNCTION(BlueprintNativeEvent)
	void SetGold(int32 InGold);

	UFUNCTION(BlueprintNativeEvent)
	void AddToItem(FGameplayTag ItemTag);

	UFUNCTION(BlueprintNativeEvent)
	void RemoveItem(FGameplayTag ItemTag);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ShowSKillCircle(UMaterialInterface* DecalMaterial = nullptr);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void HideSkillCircle();

	UFUNCTION(BlueprintNativeEvent)
	void AddItemToStoreMenu(const FGameplayTag& ItemTag, int32 ItemAmount);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SaveProgress(const FName& CheckpointTag);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void LoadProgress();
};
