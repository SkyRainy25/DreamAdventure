// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamAssetManager.h"

#include "DreamGameplayTags.h"
#include "AbilitySystemGlobals.h"

UDreamAssetManager& UDreamAssetManager::Get()
{
	check(GEngine);
	
	// �������� AssetManager�� ������ cast �Ͽ� return
	UDreamAssetManager* DreamAssetManager = Cast<UDreamAssetManager>(GEngine->AssetManager);
	return *DreamAssetManager;
}

void UDreamAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	// AssetManger�� �ε��� �ϸ�, GameplayTag�� ���
	FDreamGameplayTags::InitializeNativeGameplayTags();
	// This is required to use Target Data!
	UAbilitySystemGlobals::Get().InitGlobalData();
}
