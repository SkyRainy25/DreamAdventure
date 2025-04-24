// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "DreamAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class DREAMADVENTURE_API UDreamAssetManager : public UAssetManager
{
	GENERATED_BODY()
public:

	static UDreamAssetManager& Get();

protected:

	virtual void StartInitialLoading() override;
};
