// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/DreamWidgetController.h"

#include "Player/DreamPlayerController.h"
#include "Player/DreamPlayerState.h"
#include "AbilitySystem/DreamAbilitySystemComponent.h"
#include "AbilitySystem/DreamAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "AbilitySystem/Data/ItemInfo.h"

void UDreamWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& WCParams)
{
	PlayerController = WCParams.PlayerController;
	PlayerState = WCParams.PlayerState;
	AbilitySystemComponent = WCParams.AbilitySystemComponent;
	AttributeSet = WCParams.AttributeSet;
}

void UDreamWidgetController::BroadcastInitialValues()
{

}

void UDreamWidgetController::BindCallbacksToDependencies()
{

}

// Broadcaste Ability Info for Abilities
void UDreamWidgetController::BroadcastAbilityInfo()
{
	//TODO Get information about all given abilities, look up their Ability Info, and broadcast it to widgets.
	// Ability�� Ȱ��ȭ �Ǿ��� ��츸 ����.
	if (!GetDreamASC()->bStartupAbilitiesGiven) return;

	FForEachAbility BroadcastDelegate;
	BroadcastDelegate.BindLambda([this](const FGameplayAbilitySpec& AbilitySpec)
		{
			//TODO need a way to figure out the ability tag for a given ability spec.
			// AbilitySpec-AbilityTag�� �´� Info�� ã��, Info�� InputTag�� ä����.
			FDreamAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(DreamAbilitySystemComponent->GetAbilityTagFromSpec(AbilitySpec));
			Info.InputTag = DreamAbilitySystemComponent->GetInputTagFromSpec(AbilitySpec);
			// AbilitySpec�� Status�� ����.
			Info.StatusTag = DreamAbilitySystemComponent->GetStatusFromSpec(AbilitySpec);
			AbilityInfoDelegate.Broadcast(Info);	// �������Ʈ�� Broadcast.
		});
	GetDreamASC()->ForEachAbility(BroadcastDelegate);
}

ADreamPlayerController* UDreamWidgetController::GetDreamPC()
{
	if (DreamPlayerController == nullptr)
	{
		DreamPlayerController = Cast<ADreamPlayerController>(PlayerController);
	}
	return DreamPlayerController;
}

ADreamPlayerState* UDreamWidgetController::GetDreamPS()
{
	if (DreamPlayerState == nullptr)
	{
		DreamPlayerState = Cast<ADreamPlayerState>(PlayerState);
	}
	return DreamPlayerState;
}

UDreamAbilitySystemComponent* UDreamWidgetController::GetDreamASC()
{
	if (DreamAbilitySystemComponent == nullptr)
	{
		DreamAbilitySystemComponent = Cast<UDreamAbilitySystemComponent>(AbilitySystemComponent);
	}
	return DreamAbilitySystemComponent;
}

UDreamAttributeSet* UDreamWidgetController::GetDreamAS()
{
	if (DreamAttributeSet == nullptr)
	{
		DreamAttributeSet = Cast<UDreamAttributeSet>(AttributeSet);
	}
	return DreamAttributeSet;
}
