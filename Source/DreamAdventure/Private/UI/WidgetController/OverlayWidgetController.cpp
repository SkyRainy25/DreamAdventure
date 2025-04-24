// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/OverlayWidgetController.h"

#include "AbilitySystem/DreamAbilitySystemComponent.h"
#include "AbilitySystem/DreamAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "Player/DreamPlayerState.h"

#include "DreamGameplayTags.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	//const UDreamAttributeSet* DreamAttributeSet = CastChecked<UDreamAttributeSet>(AttributeSet);
	OnHealthChanged.Broadcast(GetDreamAS()->GetHealth());
	OnMaxHealthChanged.Broadcast(GetDreamAS()->GetMaxHealth());
	OnManaChanged.Broadcast(GetDreamAS()->GetMana());
	OnMaxManaChanged.Broadcast(GetDreamAS()->GetMaxMana());
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	//ADreamPlayerState* DreamPS = CastChecked<ADreamPlayerState>(PlayerState);
	GetDreamPS()->OnXPChangedDelegate.AddUObject(this, &UOverlayWidgetController::OnXPChanged);
	GetDreamPS()->OnLevelChangedDelegate.AddLambda(
		[this](int32 NewLevel, bool bLevelUp)
		{
			OnPlayerLevelChangedDelegate.Broadcast(NewLevel, bLevelUp);
		}
	);

	//const UDreamAttributeSet* DreamAttributeSet = CastChecked<UDreamAttributeSet>(AttributeSet);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		GetDreamAS()->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			}
	);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		GetDreamAS()->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			}
	);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		GetDreamAS()->GetManaAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnManaChanged.Broadcast(Data.NewValue);
			}
	);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		GetDreamAS()->GetMaxManaAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxManaChanged.Broadcast(Data.NewValue);
			}
	);
	if (GetDreamASC())
	{
		// Ability�� �����Ǹ� �ݹ��� �Լ� ���ε�.
		GetDreamASC()->AbilityEquipped.AddUObject(this, &UOverlayWidgetController::OnAbilityEquipped);
		if (GetDreamASC()->bStartupAbilitiesGiven)	// Ability�� �̹� Given? Ȱ��ȭ
		{
			BroadcastAbilityInfo();
		}
		else // ���� ASC���� Ability�� Given(Ȱ��ȭ) ���� �ʾҴٸ�?
		{
			// Ȱ��ȭ�Ǿ��ٴ� ���� Broadcast������ �޾Ƽ� ����.
			GetDreamASC()->AbilitiesGivenDelegate.AddUObject(this, &UOverlayWidgetController::BroadcastAbilityInfo);
		}

		GetDreamASC()->EffectAssetTags.AddLambda(
			[this](const FGameplayTagContainer& AssetTags)
			{
				for (const FGameplayTag& Tag : AssetTags)
				{
					// For example, say that Tag = Message.HealthPotion
					// "Message.HealthPotion".MatchesTag("Message") will return True, "Message".MatchesTag("Message.HealthPotion") will return False
					FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));	// MessageTag = Message
					if (Tag.MatchesTag(MessageTag))	// Tag : Message. (Tag) 
					{
						const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, Tag);
						MessageWidgetRowDelegate.Broadcast(*Row);
					}
				}
			}
		);
	}

	
	//AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
	//	UDreamAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &UOverlayWidgetController::MaxHealthChanged);

	//AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
	//	UDreamAttributeSet::GetManaAttribute()).AddUObject(this, &UOverlayWidgetController::ManaChanged);

	//AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
	//	UDreamAttributeSet::GetMaxManaAttribute()).AddUObject(this, &UOverlayWidgetController::MaxManaChanged);
}

void UOverlayWidgetController::OnXPChanged(int32 NewXP)
{
	//const ADreamPlayerState* DreamPlayerState = CastChecked<ADreamPlayerState>(PlayerState);
	const ULevelupInfo* LevelUpInfo = GetDreamPS()->LevelUpInfo;
	checkf(LevelUpInfo, TEXT("Unabled to find LevelUpInfo. Please fill out AuraPlayerState Blueprint"));

	const int32 Level = LevelUpInfo->FindLevelForXP(NewXP);
	const int32 MaxLevel = LevelUpInfo->LevelUpInformation.Num();

	if (Level <= MaxLevel && Level > 0)
	{
		const int32 LevelUpRequirement = LevelUpInfo->LevelUpInformation[Level].LevelUpRequirement;
		const int32 PreviousLevelUpRequirement = LevelUpInfo->LevelUpInformation[Level - 1].LevelUpRequirement;

		const int32 DeltaLevelRequirement = LevelUpRequirement - PreviousLevelUpRequirement;
		const int32 XPForThisLevel = NewXP - PreviousLevelUpRequirement;

		const float XPBarPercent = static_cast<float>(XPForThisLevel) / static_cast<float>(DeltaLevelRequirement);

		OnXPPercentChangedDelegate.Broadcast(XPBarPercent);
	}
}

// SkillMenuWidgetController ���� Ability�� �������� ��, Viewport�� Overlay���� �ݿ��ϱ� ���� �Լ�.
void UOverlayWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot) const
{
	const FDreamGameplayTags& GameplayTags = FDreamGameplayTags::Get();

	FDreamAbilityInfo LastSlotInfo;
	LastSlotInfo.StatusTag = GameplayTags.Abilities_Status_Unlocked;
	LastSlotInfo.InputTag = PreviousSlot;
	LastSlotInfo.AbilityTag = GameplayTags.Abilities_None;
	// Broadcast empty info if PreviousSlot is a valid slot. Only if equipping an already-equipped spell
	AbilityInfoDelegate.Broadcast(LastSlotInfo);

	FDreamAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	Info.StatusTag = Status;
	Info.InputTag = Slot;
	AbilityInfoDelegate.Broadcast(Info);
}
