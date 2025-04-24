// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/DreamAbilitySystemComponent.h"

#include "DreamGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/Abilities/DreamGameplayAbility.h"
#include "Interaction/PlayerInterface.h"
#include "AbilitySystem/DreamAbilitySystemLibrary.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Game/LoadScreenSaveGame.h"

void UDreamAbilitySystemComponent::AbilityActorInfoSet()
{
	// GE가 적용되면 콜백함수 호출 + Dynamic Multicast 가 아니기 때문에 AddUObject 사용
	// OnGameplayEffectAppliedDelegateToSelf : GE가 적용되었음을 Server에서 알리기 때문에, Client에 Replicate를 해주어야함
	// ClientEffectApplied 함수의 UFUNCTION(Client, Reliable)이라는 함수를 통해 Replicate
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UDreamAbilitySystemComponent::ClientEffectApplied);
}

void UDreamAbilitySystemComponent::AddCharacterAbilitiesFromSaveData(ULoadScreenSaveGame* SaveData)
{
	// Save 파일로부터 GA를 불러오는 함수.
	for (const FSavedAbility& Data : SaveData->SavedAbilities)
	{
		const TSubclassOf<UGameplayAbility> LoadedAbilityClass = Data.GameplayAbility;

		// AbilitySpec 가져오기
		FGameplayAbilitySpec LoadedAbilitySpec = FGameplayAbilitySpec(LoadedAbilityClass, Data.AbilityLevel);

		// Data - AbilitySlot & AbilityStatus를 설정.
		LoadedAbilitySpec.DynamicAbilityTags.AddTag(Data.AbilitySlot);
		LoadedAbilitySpec.DynamicAbilityTags.AddTag(Data.AbilityStatus);
		// Active skill 인 경우 -> GiveAbility
		if (Data.AbilityType == FDreamGameplayTags::Get().Abilities_Type_Active)
		{
			GiveAbility(LoadedAbilitySpec);
		}
		// Passive 스킬인 경우
		else if (Data.AbilityType == FDreamGameplayTags::Get().Abilities_Type_Passive)
		{
			if (Data.AbilityStatus.MatchesTagExact(FDreamGameplayTags::Get().Abilities_Status_Equipped))
			{
				GiveAbilityAndActivateOnce(LoadedAbilitySpec);	// 스킬 활성화
				MulticastActivatePassiveEffect(Data.AbilityTag, true);
			}
			else
			{
				GiveAbility(LoadedAbilitySpec);
			}
		}
	}
	bStartupAbilitiesGiven = true;
	AbilitiesGivenDelegate.Broadcast();
}

void UDreamAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		// AbilitySpec 생성
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const UDreamGameplayAbility* DreamAbility = Cast<UDreamGameplayAbility>(AbilitySpec.Ability))
		{
			// GameplayAbility가 있다면, GA의 StartupInputTag를 저장.
			AbilitySpec.DynamicAbilityTags.AddTag(DreamAbility->StartupInputTag);
			// Ability의 Status를 추적. ( ex. 이전(위) 코드에서 InputTag를 추가? -> 장착가능(장착중)이라는 '상태(Status)'
			AbilitySpec.DynamicAbilityTags.AddTag(FDreamGameplayTags::Get().Abilities_Status_Equipped);
			GiveAbility(AbilitySpec); // AbilitySpec을 통해 Grant Ability
		}
	}
	// Ability가 활성화되고 난 후, 델리게이트를 통해 broadcast
	bStartupAbilitiesGiven = true;
	AbilitiesGivenDelegate.Broadcast();
}

void UDreamAbilitySystemComponent::AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupPassiveAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		AbilitySpec.DynamicAbilityTags.AddTag(FDreamGameplayTags::Get().Abilities_Status_Equipped);
		GiveAbilityAndActivateOnce(AbilitySpec);
	}
}

void UDreamAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			if (AbilitySpec.IsActive())
			{
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, AbilitySpec.Handle, AbilitySpec.ActivationInfo.GetActivationPredictionKey());
			}
		}
	}
}

void UDreamAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	// AddCharacterAbilities에서
	// AbilitySpec.DynamicAbilityTags.AddTag를 통해 저장된 StartupInputTag 와 맞는 InputTag가 들어왔다면 활성화.
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			if (!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void UDreamAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag) && AbilitySpec.IsActive())
		{
			// Input이 Released 되었음을 알림.
			AbilitySpecInputReleased(AbilitySpec);
			// Send Data To Server What's doing
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec.Handle, AbilitySpec.ActivationInfo.GetActivationPredictionKey());
		}
	}
}

void UDreamAbilitySystemComponent::ForEachAbility(const FForEachAbility& Delegate)
{
	// 활성화된 Abilities를 관리하는 구조체? (추가 및 제거되는 것들을 관리 및 수정)
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		// 바인딩이 안된경우 에러 로그 출력.
		if (!Delegate.ExecuteIfBound(AbilitySpec))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to execute delegate in %hs"), __FUNCTION__);
		}
	}
}

FGameplayTag UDreamAbilitySystemComponent::GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	if (AbilitySpec.Ability)
	{
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags)
		{
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities"))))
			{
				return Tag;
			}
		}
	}
	return FGameplayTag();
}

FGameplayTag UDreamAbilitySystemComponent::GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{

	for (FGameplayTag Tag : AbilitySpec.DynamicAbilityTags)
	{
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("InputTag"))))
		{
			return Tag;
		}
	}
	return FGameplayTag();
}

FGameplayTag UDreamAbilitySystemComponent::GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (FGameplayTag Tag : AbilitySpec.DynamicAbilityTags)
	{
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Status"))))
		{
			return Tag;
		}
	}
	return FGameplayTag();
}

FGameplayTag UDreamAbilitySystemComponent::GetStatusFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetStatusFromSpec(*AbilitySpec);
	}
	return FGameplayTag();
}

bool UDreamAbilitySystemComponent::SlotIsEmpty(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilityHasSlot(AbilitySpec, Slot))
		{
			return false;
		}
	}
	return true;
}

// 해당 Ability가 Slot를 가지고 있는지(장착되어 있는지) 알려주는 함수.
bool UDreamAbilitySystemComponent::AbilityHasSlot(const FGameplayAbilitySpec& Spec, const FGameplayTag& Slot)
{
	return Spec.DynamicAbilityTags.HasTagExact(Slot);
}

// Ability의 InputTag가 있는지 알려주는 함수.
bool UDreamAbilitySystemComponent::AbilityHasAnySlot(const FGameplayAbilitySpec& Spec)
{
	return Spec.DynamicAbilityTags.HasTag(FGameplayTag::RequestGameplayTag(FName("InputTag")));
}

// 해당 슬롯에 있는 Ability의 Spec을 가져오는 함수.
FGameplayAbilitySpec* UDreamAbilitySystemComponent::GetSpecWithSlot(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(Slot))
		{
			return &AbilitySpec;
		}
	}
	return nullptr;
}

bool UDreamAbilitySystemComponent::IsPassiveAbility(const FGameplayAbilitySpec& Spec) const
{
	const UAbilityInfo* AbilityInfo = UDreamAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	const FGameplayTag AbilityTag = GetAbilityTagFromSpec(Spec);
	const FDreamAbilityInfo& Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	const FGameplayTag& AbilityType = Info.AbilityType;
	return AbilityType.MatchesTagExact(FDreamGameplayTags::Get().Abilities_Type_Passive);
}

void UDreamAbilitySystemComponent::AssignSlotToAbility(FGameplayAbilitySpec& Spec, const FGameplayTag& Slot)
{
	// 장착하려는 스킬의 이전 슬롯을 제거
	ClearSlot(&Spec);
	// 장착하려는 스킬에 새로운 Slot을 부여.
	Spec.DynamicAbilityTags.AddTag(Slot);
}

void UDreamAbilitySystemComponent::MulticastActivatePassiveEffect_Implementation(const FGameplayTag& AbilityTag, bool bActivate)
{
	ActivatePassiveEffect.Broadcast(AbilityTag, bActivate);
}

FGameplayTag UDreamAbilitySystemComponent::GetSlotFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetInputTagFromSpec(*AbilitySpec);
	}
	return FGameplayTag();
}

FGameplayAbilitySpec* UDreamAbilitySystemComponent::GetSpecFromAbilityTag(const FGameplayTag& AbilityTag)
{
	FScopedAbilityListLock ActiveScopeLoc(*this);
	TArray<FGameplayAbilitySpec> Temp = GetActivatableAbilities();
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags)
		{
			bool ret = Tag.MatchesTag(AbilityTag);
			if (Tag == AbilityTag)
			{
				return &AbilitySpec;
			}
		}
	}
	return nullptr;
}

void UDreamAbilitySystemComponent::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		// AttributePoints 가 있으면? 
		if (IPlayerInterface::Execute_GetAttributePoints(GetAvatarActor()) > 0)
		{
			// 서버에서 처리
			ServerUpgradeAttribute(AttributeTag);
		}
	}
}

void UDreamAbilitySystemComponent::ServerUpgradeAttribute_Implementation(const FGameplayTag& AttributeTag)
{
	FGameplayEventData Payload;
	Payload.EventTag = AttributeTag;
	Payload.EventMagnitude = 1.f;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, Payload);
	
	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		// AttributePoints를 사용할 경우 차감.
		IPlayerInterface::Execute_AddToAttributePoints(GetAvatarActor(), -1);
	}
}

// 필요 레벨을 넘겼지만, Grant되지 않는 Ability를 활성화 해주는 함수.
void UDreamAbilitySystemComponent::UpdateAbilityStatuses(int32 Level)
{
	UAbilityInfo* AbilityInfo = UDreamAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	for (const FDreamAbilityInfo Info : AbilityInfo->AbilityInformation)	// AbilityInformation에 있는 모든 GA를 순회.
	{
		if (!Info.AbilityTag.IsValid()) continue;	// AbilityTag가 없는 경우 -> 패스
		if (Level < Info.LevelRequirement) continue;	// 필요 레벨이 안되는 경우 -> 패스
		// AbilitySpec이 없는 경우? -> AbilitySpec 생성.
		if (GetSpecFromAbilityTag(Info.AbilityTag) == nullptr)
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Info.Ability, 1);
			// 필요레벨에 도달 -> Status : Eligible
			AbilitySpec.DynamicAbilityTags.AddTag(FDreamGameplayTags::Get().Abilities_Status_Eligible);
			GiveAbility(AbilitySpec);
			MarkAbilitySpecDirty(AbilitySpec);	// Force AbilitySpec To be Replicated 
			ClientUpdateAbilityStatus(Info.AbilityTag, FDreamGameplayTags::Get().Abilities_Status_Eligible, 1);
		}
	}
}

void UDreamAbilitySystemComponent::EquipItem(const FGameplayTag& AttributeTag, FGameplayTag& ItemTag, FGameplayTag& StatusTag, int32 Magnitude)
{
	const FDreamGameplayTags Tags = FDreamGameplayTags::Get();

	if (StatusTag != Tags.Item_Status_Equipped) return;
	// 장착할 아이템의 능력치 적용
	FGameplayEventData Payload;
	Payload.EventTag = AttributeTag;
	Payload.EventMagnitude = Magnitude;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, Payload);

}

void UDreamAbilitySystemComponent::RemoveItem(const FGameplayTag& AttributeTag, const FGameplayTag& StatusTag, int32 Magnitude)
{
	// 장착중인 아이템이 아니라면 종료.
	if (StatusTag == FDreamGameplayTags::Get().Item_Status_Normal)	return;

	FGameplayEventData PrevPayload;
	PrevPayload.EventTag = AttributeTag;
	PrevPayload.EventMagnitude = -Magnitude; // - 부호를 붙여 제거할 때, 수치만큼 감소 후 증가.
	// 장착 중이던 능력치를 감소(제거).
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, PrevPayload);

}

void UDreamAbilitySystemComponent::UseItem(const FGameplayTag& AttributeTag, const FGameplayTag& StatusTag, int32 Magnitude)
{
	FGameplayEventData Payload;
	Payload.EventTag = AttributeTag;
	Payload.EventMagnitude = Magnitude;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, Payload);
}

void UDreamAbilitySystemComponent::ServerSpendSkillPoint_Implementation(const FGameplayTag& AbilityTag)
{
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		if (GetAvatarActor()->Implements<UPlayerInterface>())
		{
			IPlayerInterface::Execute_AddToSkillPoints(GetAvatarActor(), -1);
		}

		const FDreamGameplayTags GameplayTags = FDreamGameplayTags::Get();
		FGameplayTag Status = GetStatusFromSpec(*AbilitySpec);
		if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
		{
			// Eligible 태그를 지우고 Unlocked 태그 추가 -> Tag를 통한 Status 변경
			AbilitySpec->DynamicAbilityTags.RemoveTag(GameplayTags.Abilities_Status_Eligible);
			AbilitySpec->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Unlocked);
			Status = GameplayTags.Abilities_Status_Unlocked;	// Status Update

		}
		if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Equipped) || Status.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
		{
			AbilitySpec->Level += 1;
		}
		ClientUpdateAbilityStatus(AbilityTag, Status, AbilitySpec->Level);	// Updatg한 Status를 Client에 업데이트
		MarkAbilitySpecDirty(*AbilitySpec);	// Force AbilitySpec To be Replicated 
	}
}

// 2 SkillRowBoxPressed를 통해 스킬을 장착하는 과정 수행
void UDreamAbilitySystemComponent::ServerEquipAbility_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& Slot)
{
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		const FDreamGameplayTags& GameplayTags = FDreamGameplayTags::Get();
		const FGameplayTag& PrevSlot = GetInputTagFromSpec(*AbilitySpec);
		const FGameplayTag& Status = GetStatusFromSpec(*AbilitySpec);

		const bool bValidStatus = Status == GameplayTags.Abilities_Status_Equipped || Status == GameplayTags.Abilities_Status_Unlocked;
		if (bValidStatus)
		{
			// Handle activation/deactivation for passive abilities

			// 이미 장착된 스킬이 있는 경우.
			if (!SlotIsEmpty(Slot))  // There is an ability in this slot already. Deactivate and clear its slot.
			{
				// 해당 슬롯에 있는 Ability
				FGameplayAbilitySpec* SpecWithSlot = GetSpecWithSlot(Slot);
				if (SpecWithSlot)
				{
					// 장착하려는 스킬 = 장착중인 스킬일 경우. => early return.
					// is that ability the same as this ability? If so, we can return early.
					if (AbilityTag.MatchesTagExact(GetAbilityTagFromSpec(*SpecWithSlot)))
					{
						ClientEquipAbility(AbilityTag, GameplayTags.Abilities_Status_Equipped, Slot, PrevSlot);
						return;
					}

					// 이미 장착중인 PassiveAbility 를 Deactivate하기 위해 델리게이트를 Broadcast
					if (IsPassiveAbility(*SpecWithSlot))
					{
						// 기존에 활성화된 PassiveAbility를 DeActivate(false)
						// ActivatePassiveEffect(Tag, false)를 넘김.
						MulticastActivatePassiveEffect(GetAbilityTagFromSpec(*SpecWithSlot), false);
						DeactivatePassiveAbility.Broadcast(GetAbilityTagFromSpec(*SpecWithSlot));
					}
					ClearSlot(SpecWithSlot);
				}
			}

			// 새로운 Ability를 장착(Slot에 장착되지 않았다면)
			if (!AbilityHasAnySlot(*AbilitySpec)) // Ability doesn't yet have a slot (it's not active)
			{
				if (IsPassiveAbility(*AbilitySpec))
				{
					TryActivateAbility(AbilitySpec->Handle);
					// 새로운 Passive Ability를 활성화(true)
					// ActivatePassiveEffect(Tag, true)를 넘김.
					MulticastActivatePassiveEffect(AbilityTag, true);
				}
				AbilitySpec->DynamicAbilityTags.RemoveTag(GetStatusFromSpec(*AbilitySpec));
				AbilitySpec->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Equipped);
			}
			AssignSlotToAbility(*AbilitySpec, Slot);
			MarkAbilitySpecDirty(*AbilitySpec);

			/*
			// Remove this InputTag (slot) from any Ability that has it.
			// 배정할 슬롯에 Ability가 이미 존재한다면, Ability를 지워주고
			ClearAbilitiesOfSlot(Slot);
			// Clear this ability's slot, just in case, it's a different slot
			// 기존에 있던 Ability가 가진 InputTag를 지움.
			ClearSlot(AbilitySpec);
			// Now, assign this ability to this slot
			// 새로 배정할 Ability의 InputTag(Slot)을	추가
			AbilitySpec->DynamicAbilityTags.AddTag(Slot);
			if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
			{
				AbilitySpec->DynamicAbilityTags.RemoveTag(GameplayTags.Abilities_Status_Unlocked);
				AbilitySpec->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Equipped);
			}
			MarkAbilitySpecDirty(*AbilitySpec);
			*/
		}
		// 3 Cached된 정보를 없애기 전에 Client에 알림.
		ClientEquipAbility(AbilityTag, GameplayTags.Abilities_Status_Equipped, Slot, PrevSlot);
	}
}

void UDreamAbilitySystemComponent::ClientEquipAbility_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot)
{
	//4 Broadcast
	AbilityEquipped.Broadcast(AbilityTag, Status, Slot, PreviousSlot);
}

bool UDreamAbilitySystemComponent::GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription, FString& OutNextLevelDescription)
{
	if (const FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		if (UDreamGameplayAbility* DreamAbility = Cast < UDreamGameplayAbility>(AbilitySpec->Ability))
		{
			OutDescription = DreamAbility->GetDescription(AbilitySpec->Level);
			OutNextLevelDescription = DreamAbility->GetNextLevelDescription(AbilitySpec->Level + 1);
			return true;
		}
	}
	const UAbilityInfo* AbilityInfo = UDreamAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	if (!AbilityTag.IsValid() || AbilityTag.MatchesTagExact(FDreamGameplayTags::Get().Abilities_Status_Locked))
	{
		OutDescription = FString();
	}
	else
	{
		OutDescription = UDreamGameplayAbility::GetLockedDescription(AbilityInfo->FindAbilityInfoForTag(AbilityTag).LevelRequirement);
	}
	OutNextLevelDescription = FString();
	return false;
}

void UDreamAbilitySystemComponent::ClearSlot(FGameplayAbilitySpec* Spec)
{
	const FGameplayTag& Slot = GetInputTagFromSpec(*Spec);
	Spec->DynamicAbilityTags.RemoveTag(Slot);
}

void UDreamAbilitySystemComponent::ClearAbilitiesOfSlot(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	// 활성화된 Ability중 해당 Slot을 가지고 있는 Ability를 Clear
	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (AbilityHasSlot(&Spec, Slot))
		{
			ClearSlot(&Spec);
		}
	}
}

// AbilitySpec이 해당 Slot을 가지고 있는지 판별하는 함수
bool UDreamAbilitySystemComponent::AbilityHasSlot(FGameplayAbilitySpec* Spec, const FGameplayTag& Slot)
{
	for (FGameplayTag Tag : Spec->DynamicAbilityTags)
	{
		if (Tag.MatchesTagExact(Slot))
		{
			return true;
		}
	}
	return false;
}

void UDreamAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	// 처음에 클라이언트에서는 false이기 때문에 처음만 설정.
	if (!bStartupAbilitiesGiven)
	{
		// true로 해야 OnInitializeStartupAbilities() 함수 실행 가능.
		bStartupAbilitiesGiven = true;
		AbilitiesGivenDelegate.Broadcast();
	}
}

void UDreamAbilitySystemComponent::ClientUpdateAbilityStatus_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 AbilityLevel)
{
	AbilityStatusChanged.Broadcast(AbilityTag, StatusTag, AbilityLevel);
}

//void UDreamAbilitySystemComponent::ClientEffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
//{
//	FGameplayTagContainer TagContainer;
//	EffectSpec.GetAllAssetTags(TagContainer);
//
//	EffectAssetTags.Broadcast(TagContainer);
//}

void UDreamAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle) 
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);

	EffectAssetTags.Broadcast(TagContainer);
}
