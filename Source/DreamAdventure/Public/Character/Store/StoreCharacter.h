// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameplayTagContainer.h"
#include "StoreCharacter.generated.h"

class UDreamUserWidget;

DECLARE_MULTICAST_DELEGATE_TwoParams(FStoreItemAddSignature, const FGameplayTag&, int32);

UCLASS()
class DREAMADVENTURE_API AStoreCharacter : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AStoreCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	TArray<FGameplayTag> StoreItems;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSubclassOf<UDreamUserWidget> StoreWidgetClass;

	//TObjectPtr<class StoreClass> Store;

	FStoreItemAddSignature OnStoreItemAddDelegate;

	UFUNCTION(BlueprintCallable)
	void OnOverlap(AActor* TargetActor, TMap<FGameplayTag, int32> ItemPack );
};
