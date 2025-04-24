// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/AttributeMenuWidgetController.h"
#include "AbilitySystem/DreamAbilitySystemComponent.h"
#include "AbilitySystem/DreamAttributeSet.h"
#include "AbilitySystem/Data/AttributeInfo.h"
#include "DreamGameplayTags.h"
#include "Player/DreamPlayerState.h"

void UAttributeMenuWidgetController::BindCallbacksToDependencies()
{
	//UDreamAttributeSet* AS = CastChecked<UDreamAttributeSet>(AttributeSet);
	check(AttributeInfo);

	for (auto& Pair : GetDreamAS()->TagsToAttributes)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda(
			[this, Pair](const FOnAttributeChangeData& Data)
			{
				BroadcastAttributeInfo(Pair.Key, Pair.Value());
			}
		);
	}

	//ADreamPlayerState* DreamPS = CastChecked<ADreamPlayerState>(PlayerState);
	GetDreamPS()->OnAttributePointsChangedDelegate.AddLambda(
		[this](int32 Points)
		{
			AttributePointsChangedDelegate.Broadcast(Points);
		}
	);
}

void UAttributeMenuWidgetController::BroadcastInitialValues()
{
	UDreamAttributeSet* AS = CastChecked<UDreamAttributeSet>(AttributeSet);
	check(AttributeInfo);

	for (auto& Pair : AS->TagsToAttributes)
	{
		BroadcastAttributeInfo(Pair.Key, Pair.Value());
	}

	//ADreamPlayerState* DreamPS = CastChecked<ADreamPlayerState>(PlayerState);
	AttributePointsChangedDelegate.Broadcast(GetDreamPS()->GetAttributePoints());
}

void UAttributeMenuWidgetController::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	UDreamAbilitySystemComponent* DreamASC = CastChecked<UDreamAbilitySystemComponent>(AbilitySystemComponent);
	DreamASC->UpgradeAttribute(AttributeTag);
}

void UAttributeMenuWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute) const
{
	FDreamAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag);
	Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);	// Value만 코드상에서 설정
	// BP_AttrobiteWC 에서 UAttributeInfo타입의 DA_AttributeInfo가 들어가 있기 때문에
	AttributeInfoDelegate.Broadcast(Info);	// 나머지 데이터는 DA_AttributeInfo에서 가져와서 설정.
}
