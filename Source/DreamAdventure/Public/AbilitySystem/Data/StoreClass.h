// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagConTainer.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FStoreItemChanged, const FGameplayTag& ItemTag, int32 ItemAmount)

/**
 * 
 */
class DREAMADVENTURE_API StoreClass
{
public:
	StoreClass();
	~StoreClass();

	FStoreItemChanged OnStoreItemChangedDelegate;

public:
	FGameplayTag ItemTag = FGameplayTag();

	int32 ItemAmount = 1;

	int32 ItemPrice = 1;
};
