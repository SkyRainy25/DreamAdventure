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
				// AbilityTag에 해당하는 AbilityInfo를 찾고
				FDreamAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
				// Ability의 StatusTag 를 InputParams로 받은 StatusTag(Eligible)로 변경.
				Info.StatusTag = StatusTag;
				AbilityInfoDelegate.Broadcast(Info);
			}
		}
	);

	// 5 Client를 통해 Broadcast -> Binding
	GetDreamASC()->AbilityEquipped.AddUObject(this, &USkillMenuWidgetController::OnAbilityEquipped);

	GetDreamPS()->OnSkillPointsChangedDelegate.AddLambda(
		[this](int32 SkillPoints)
		{
			SkillPointsChanged.Broadcast(SkillPoints);
			CurrentSkillPoints = SkillPoints;

			bool bEnableSpendPoints = false;
			bool bEnableEquip = false;
			// StatusTag를 InputParameter로 받지 않기 때문에, Tracking에 사용한 SelectedAbility를 이용해 StatusTag를 가져옴.
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
		// AbilitySpec도 있고 Tag도 Valid하며, Tag도 있는 경우.
		// -> Spec에서 Status Tag를 가져옴.
		AbilityStatus = GetDreamASC()->GetStatusFromSpec(*AbilitySpec);
	}

	// 클릭한 스킬의 Status와 AbilityTag를 저장. -> BindCallbackToDependencies에서 사용.
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
		// 현재 슬롯에 있는 스킬의 InputTag(Slot)을 임시 저장.
		SelectedSlot = GetDreamASC()->GetSlotFromAbilityTag(SelectedStatus);
	}
}

void USkillMenuWidgetController::SkillRowBoxPressed(const FGameplayTag& SlotTag, const FGameplayTag& AbilityType)
{
	if (!bWaitingForEquipSelection) return;
	// Active(Passive) Skill - Active(Passive)Skill Slot에 있는지를 확인하기 위함.
	// Check selected ability against the slot's ability type.
	// (don't equip an offensive spell in a passive slot and vice versa)
	const FGameplayTag& SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(AbilityType).AbilityType;
	if (SelectedAbilityType.MatchesTagExact(AbilityType))	return;

	// 1. ASC를 통해 서버에서 관련 작업 수행
	GetDreamASC()->ServerEquipAbility(SelectedAbility.Ability, SlotTag);
}

// 이전 스킬의 정보를 지우고, 새로운 스킬의 정보를 저장.
void USkillMenuWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot)
{
	bWaitingForEquipSelection = false;

	const FDreamGameplayTags& GameplayTags = FDreamGameplayTags::Get();

	// 이전 Ability에 관한 Info를 지우고
	FDreamAbilityInfo LastSlotInfo;
	LastSlotInfo.StatusTag = GameplayTags.Abilities_Status_Unlocked;
	LastSlotInfo.InputTag = PreviousSlot;
	LastSlotInfo.AbilityTag = GameplayTags.Abilities_None;
	// Broadcast empty info if PreviousSlot is a valid slot. Only if equipping an already-equipped spell
	AbilityInfoDelegate.Broadcast(LastSlotInfo);

	// 새로 배정된 Ability의 Info를 채워넣음.
	FDreamAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	Info.StatusTag = Status;
	Info.InputTag = Slot;
	AbilityInfoDelegate.Broadcast(Info);

	// 관련 애니메이션 재생 중단을 위한 Broadcast
	StopWaitingForEquipDelegate.Broadcast(AbilityInfo->FindAbilityInfoForTag(AbilityTag).AbilityType);
	SkillBoxReassignedDelegate.Broadcast(AbilityTag);
	SkillBoxDeselect();
}

void USkillMenuWidgetController::ShouldEnableButtons(const FGameplayTag& AbilityStatus, int32 SkillPoints, bool& bShouldEnableSpellPointsButton, bool& bShouldEnableEquipButton)
{
	const FDreamGameplayTags  GameplayTags = FDreamGameplayTags::Get();

	bShouldEnableSpellPointsButton = false;
	bShouldEnableEquipButton = false;

	// 이미 장착중일 경우, 포인트 소모 가능 / 스킬 장착 가능
	if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Equipped))
	{
		bShouldEnableEquipButton = true;
		if (SkillPoints > 0)
		{
			bShouldEnableSpellPointsButton = true;
		}
	}
	// Eligible -> 열리지도 장착중이지도 않은 상태 => 스킬 포인트만 소비 가능
	else if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
	{
		if (SkillPoints > 0)
		{
			bShouldEnableSpellPointsButton = true;
		}
	}
	// 열린 경우 -> 스킬 포인트 소모 가능 + 장착 가능
	else if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
	{
		bShouldEnableEquipButton = true;
		if (SkillPoints > 0)
		{
			bShouldEnableSpellPointsButton = true;
		}
	}
}

