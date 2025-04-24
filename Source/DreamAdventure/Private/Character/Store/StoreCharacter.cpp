// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Store/StoreCharacter.h"

#include "UI/Widget/DreamUserWidget.h"
#include "AbilitySystem/Data/StoreClass.h"
#include "Character/PlayerCharacter.h"
#include "AbilitySystem/Data/ItemInfo.h"
#include "AbilitySystem/DreamAbilitySystemLibrary.h"
#include "UI/WidgetController/StoreWidgetController.h"

// Sets default values
AStoreCharacter::AStoreCharacter()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AStoreCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AStoreCharacter::OnOverlap(AActor* TargetActor, TMap<FGameplayTag, int32> ItemPack)
{
	// �÷��̾ �ƴ� ��� ���� ����
	if (!TargetActor->Implements<UPlayerInterface>())	return;

	// ItemTag - ItemAmount�� UE ���� ����.
	for (TPair Item : ItemPack)
	{
		IPlayerInterface::Execute_AddItemToStoreMenu(TargetActor, Item.Key, Item.Value);
	}

}

