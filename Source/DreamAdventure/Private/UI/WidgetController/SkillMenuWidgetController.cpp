// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/SkillMenuWidgetController.h"

#include "AbilitySystem/DreamAbilitySystemComponent.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Player/DreamPlayerState.h"

void USkillMenuWidgetController::BroadcastInitialValues()
{
	BroadcastAbilityInfo();
	SkillPointsChanged.Broadcast(GetDreamPS()->GetSkillPoints());
}

void USkillMenuWidgetController::BindCallbacksToDependencies()
{
	GetDreamASC()->AbilityStatusChanged.AddLambda(
		[this](const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 NewLevel)
		{
			if (SelectedAbility.Ability.MatchesTagExact(AbilityTag))
			{
				SelectedAbility.Status = StatusTag;
				bool bEnableSpendPoints = false;
				bool bEnableEquip = false;
				ShouldEnableButtons(StatusTag, CurrentSkillPoints, bEnableSpendPoints, bEnableEquip);
				FString Description;
				FString NextLevelDescription;
				GetDreamASC()->GetDescriptionsByAbilityTag(AbilityTag, Description, NextLevelDescription);
				SkillBoxSelectedDelegate.Broadcast(bEnableSpendPoints, bEnableEquip, Description, NextLevelDescription);
			}
			if (AbilityInfo)
			{
				// AbilityTag�� �ش��ϴ� AbilityInfo�� ã��
				FDreamAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
				// Ability�� StatusTag �� InputParams�� ���� StatusTag(Eligible)�� ����.
				Info.StatusTag = StatusTag;
				AbilityInfoDelegate.Broadcast(Info);
			}
		}
	);

	// 5 Client�� ���� Broadcast -> Binding
	GetDreamASC()->AbilityEquipped.AddUObject(this, &USkillMenuWidgetController::OnAbilityEquipped);

	GetDreamPS()->OnSkillPointsChangedDelegate.AddLambda(
		[this](int32 SkillPoints)
		{
			SkillPointsChanged.Broadcast(SkillPoints);
			CurrentSkillPoints = SkillPoints;

			bool bEnableSpendPoints = false;
			bool bEnableEquip = false;
			// StatusTag�� InputParameter�� ���� �ʱ� ������, Tracking�� ����� SelectedAbility�� �̿��� StatusTag�� ������.
			ShouldEnableButtons(SelectedAbility.Status, CurrentSkillPoints, bEnableSpendPoints, bEnableEquip);
			FString Description;
			FString NextLevelDescription;
			GetDreamASC()->GetDescriptionsByAbilityTag(SelectedAbility.Ability, Description, NextLevelDescription);
			SkillBoxSelectedDelegate.Broadcast(bEnableSpendPoints, bEnableEquip, Description, NextLevelDescription);
		}
	);
}

void USkillMenuWidgetController::SkillBoxSelected(const FGameplayTag& AbilityTag)
{
	if (bWaitingForEquipSelection)
	{
		const FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(AbilityTag).AbilityType;
		StopWaitingForEquipDelegate.Broadcast(SelectedAbilityType);
		bWaitingForEquipSelection = false;

	}
	const FDreamGameplayTags  GameplayTags = FDreamGameplayTags::Get();
	const int32 SkillPoints = GetDreamPS()->GetSkillPoints();
	FGameplayTag AbilityStatus;

	const bool bTagValid = AbilityTag.IsValid();
	const bool bTagNone = AbilityTag.MatchesTag(GameplayTags.Abilities_None);
	const FGameplayAbilitySpec* AbilitySpec = GetDreamASC()->GetSpecFromAbilityTag(AbilityTag);
	const bool bSpecValid = AbilitySpec != nullptr;
	if (!bTagValid || bTagNone || !bSpecValid)
	{
		AbilityStatus = GameplayTags.Abilities_Type_None;
	}
	else
	{
		// AbilitySpec�� �ְ� Tag�� Valid�ϸ�, Tag�� �ִ� ���.
		// -> Spec���� Status Tag�� ������.
		AbilityStatus = GetDreamASC()->GetStatusFromSpec(*AbilitySpec);
	}

	// Ŭ���� ��ų�� Status�� AbilityTag�� ����. -> BindCallbackToDependencies���� ���.
	SelectedAbility.Ability = AbilityTag;
	SelectedAbility.Status = AbilityStatus;

	bool bEnableSpendPoints = false;
	bool bEnableEquip = false;
	ShouldEnableButtons(AbilityStatus, SkillPoints, bEnableSpendPoints, bEnableEquip);
	FString Description;
	FString NextLevelDescription;
	GetDreamASC()->GetDescriptionsByAbilityTag(AbilityTag, Description, NextLevelDescription);
	SkillBoxSelectedDelegate.Broadcast(bEnableSpendPoints, bEnableEquip, Description, NextLevelDescription);
	
}

void USkillMenuWidgetController::SpendPointButtonPressed()
{
	if (GetDreamASC())
	{
		GetDreamASC()->ServerSpendSkillPoint(SelectedAbility.Ability);
	}
}

void USkillMenuWidgetController::SkillBoxDeselect()
{
	if (bWaitingForEquipSelection)
	{
		const FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
		StopWaitingForEquipDelegate.Broadcast(SelectedAbilityType);
		bWaitingForEquipSelection = false;
	}
	SelectedAbility.Ability = FDreamGameplayTags::Get().Abilities_None;
	SelectedAbility.Status = FDreamGameplayTags::Get().Abilities_Status_Locked;

	SkillBoxSelectedDelegate.Broadcast(false, false, FString(), FString());
}

void USkillMenuWidgetController::EquipButtonPressed()
{
	const FGameplayTag AbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;

	WaitForEquipDelegate.Broadcast(AbilityType);
	bWaitingForEquipSelection = true;

	const FGameplayTag SelectedStatus = GetDreamASC()->GetStatusFromAbilityTag(SelectedAbility.Ability);
	if (SelectedStatus.MatchesTagExact(FDreamGameplayTags::Get().Abilities_Status_Equipped))
	{
		// ���� ���Կ� �ִ� ��ų�� InputTag(Slot)�� �ӽ� ����.
		SelectedSlot = GetDreamASC()->GetSlotFromAbilityTag(SelectedStatus);
	}
}

void USkillMenuWidgetController::SkillRowBoxPressed(const FGameplayTag& SlotTag, const FGameplayTag& AbilityType)
{
	if (!bWaitingForEquipSelection) return;
	// Active(Passive) Skill - Active(Passive)Skill Slot�� �ִ����� Ȯ���ϱ� ����.
	// Check selected ability against the slot's ability type.
	// (don't equip an offensive spell in a passive slot and vice versa)
	const FGameplayTag& SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(AbilityType).AbilityType;
	if (SelectedAbilityType.MatchesTagExact(AbilityType))	return;

	// 1. ASC�� ���� �������� ���� �۾� ����
	GetDreamASC()->ServerEquipAbility(SelectedAbility.Ability, SlotTag);
}

// ���� ��ų�� ������ �����, ���ο� ��ų�� ������ ����.
void USkillMenuWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot)
{
	bWaitingForEquipSelection = false;

	const FDreamGameplayTags& GameplayTags = FDreamGameplayTags::Get();

	// ���� Ability�� ���� Info�� �����
	FDreamAbilityInfo LastSlotInfo;
	LastSlotInfo.StatusTag = GameplayTags.Abilities_Status_Unlocked;
	LastSlotInfo.InputTag = PreviousSlot;
	LastSlotInfo.AbilityTag = GameplayTags.Abilities_None;
	// Broadcast empty info if PreviousSlot is a valid slot. Only if equipping an already-equipped spell
	AbilityInfoDelegate.Broadcast(LastSlotInfo);

	// ���� ������ Ability�� Info�� ä������.
	FDreamAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	Info.StatusTag = Status;
	Info.InputTag = Slot;
	AbilityInfoDelegate.Broadcast(Info);

	// ���� �ִϸ��̼� ��� �ߴ��� ���� Broadcast
	StopWaitingForEquipDelegate.Broadcast(AbilityInfo->FindAbilityInfoForTag(AbilityTag).AbilityType);
	SkillBoxReassignedDelegate.Broadcast(AbilityTag);
	SkillBoxDeselect();
}

void USkillMenuWidgetController::ShouldEnableButtons(const FGameplayTag& AbilityStatus, int32 SkillPoints, bool& bShouldEnableSpellPointsButton, bool& bShouldEnableEquipButton)
{
	const FDreamGameplayTags  GameplayTags = FDreamGameplayTags::Get();

	bShouldEnableSpellPointsButton = false;
	bShouldEnableEquipButton = false;

	// �̹� �������� ���, ����Ʈ �Ҹ� ���� / ��ų ���� ����
	if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Equipped))
	{
		bShouldEnableEquipButton = true;
		if (SkillPoints > 0)
		{
			bShouldEnableSpellPointsButton = true;
		}
	}
	// Eligible -> �������� ������������ ���� ���� => ��ų ����Ʈ�� �Һ� ����
	else if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
	{
		if (SkillPoints > 0)
		{
			bShouldEnableSpellPointsButton = true;
		}
	}
	// ���� ��� -> ��ų ����Ʈ �Ҹ� ���� + ���� ����
	else if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
	{
		bShouldEnableEquipButton = true;
		if (SkillPoints > 0)
		{
			bShouldEnableSpellPointsButton = true;
		}
	}
}

