// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamAssetManager.h"

#include "DreamGameplayTags.h"
#include "AbilitySystemGlobals.h"

UDreamAssetManager& UDreamAssetManager::Get()
{
	check(GEngine);
	
	// 엔진에서 AssetManager를 가져와 cast 하여 return
	UDreamAssetManager* DreamAssetManager = Cast<UDreamAssetManager>(GEngine->AssetManager);
	return *DreamAssetManager;
}

void UDreamAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	// AssetManger가 로딩을 하며, GameplayTag를 등록
	FDreamGameplayTags::InitializeNativeGameplayTags();
	// This is required to use Target Data!
	UAbilitySystemGlobals::Get().InitGlobalData();
}
