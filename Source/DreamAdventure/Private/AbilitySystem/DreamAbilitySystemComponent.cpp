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
	// GE�� ����Ǹ� �ݹ��Լ� ȣ�� + Dynamic Multicast �� �ƴϱ� ������ AddUObject ���
	// OnGameplayEffectAppliedDelegateToSelf : GE�� ����Ǿ����� Server���� �˸��� ������, Client�� Replicate�� ���־����
	// ClientEffectApplied �Լ��� UFUNCTION(Client, Reliable)�̶�� �Լ��� ���� Replicate
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UDreamAbilitySystemComponent::ClientEffectApplied);
}

void UDreamAbilitySystemComponent::AddCharacterAbilitiesFromSaveData(ULoadScreenSaveGame* SaveData)
{
	// Save ���Ϸκ��� GA�� �ҷ����� �Լ�.
	for (const FSavedAbility& Data : SaveData->SavedAbilities)
	{
		const TSubclassOf<UGameplayAbility> LoadedAbilityClass = Data.GameplayAbility;

		// AbilitySpec ��������
		FGameplayAbilitySpec LoadedAbilitySpec = FGameplayAbilitySpec(LoadedAbilityClass, Data.AbilityLevel);

		// Data - AbilitySlot & AbilityStatus�� ����.
		LoadedAbilitySpec.DynamicAbilityTags.AddTag(Data.AbilitySlot);
		LoadedAbilitySpec.DynamicAbilityTags.AddTag(Data.AbilityStatus);
		// Active skill �� ��� -> GiveAbility
		if (Data.AbilityType == FDreamGameplayTags::Get().Abilities_Type_Active)
		{
			GiveAbility(LoadedAbilitySpec);
		}
		// Passive ��ų�� ���
		else if (Data.AbilityType == FDreamGameplayTags::Get().Abilities_Type_Passive)
		{
			if (Data.AbilityStatus.MatchesTagExact(FDreamGameplayTags::Get().Abilities_Status_Equipped))
			{
				GiveAbilityAndActivateOnce(LoadedAbilitySpec);	// ��ų Ȱ��ȭ
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
		// AbilitySpec ����
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const UDreamGameplayAbility* DreamAbility = Cast<UDreamGameplayAbility>(AbilitySpec.Ability))
		{
			// GameplayAbility�� �ִٸ�, GA�� StartupInputTag�� ����.
			AbilitySpec.DynamicAbilityTags.AddTag(DreamAbility->StartupInputTag);
			// Ability�� Status�� ����. ( ex. ����(��) �ڵ忡�� InputTag�� �߰�? -> ��������(������)�̶�� '����(Status)'
			AbilitySpec.DynamicAbilityTags.AddTag(FDreamGameplayTags::Get().Abilities_Status_Equipped);
			GiveAbility(AbilitySpec); // AbilitySpec�� ���� Grant Ability
		}
	}
	// Ability�� Ȱ��ȭ�ǰ� �� ��, ��������Ʈ�� ���� broadcast
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

	// AddCharacterAbilities����
	// AbilitySpec.DynamicAbilityTags.AddTag�� ���� ����� StartupInputTag �� �´� InputTag�� ���Դٸ� Ȱ��ȭ.
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
			// Input�� Released �Ǿ����� �˸�.
			AbilitySpecInputReleased(AbilitySpec);
			// Send Data To Server What's doing
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec.Handle, AbilitySpec.ActivationInfo.GetActivationPredictionKey());
		}
	}
}

void UDreamAbilitySystemComponent::ForEachAbility(const FForEachAbility& Delegate)
{
	// Ȱ��ȭ�� Abilities�� �����ϴ� ����ü? (�߰� �� ���ŵǴ� �͵��� ���� �� ����)
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		// ���ε��� �ȵȰ�� ���� �α� ���.
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

// �ش� Ability�� Slot�� ������ �ִ���(�����Ǿ� �ִ���) �˷��ִ� �Լ�.
bool UDreamAbilitySystemComponent::AbilityHasSlot(const FGameplayAbilitySpec& Spec, const FGameplayTag& Slot)
{
	return Spec.DynamicAbilityTags.HasTagExact(Slot);
}

// Ability�� InputTag�� �ִ��� �˷��ִ� �Լ�.
bool UDreamAbilitySystemComponent::AbilityHasAnySlot(const FGameplayAbilitySpec& Spec)
{
	return Spec.DynamicAbilityTags.HasTag(FGameplayTag::RequestGameplayTag(FName("InputTag")));
}

// �ش� ���Կ� �ִ� Ability�� Spec�� �������� �Լ�.
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
	// �����Ϸ��� ��ų�� ���� ������ ����
	ClearSlot(&Spec);
	// �����Ϸ��� ��ų�� ���ο� Slot�� �ο�.
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
		// AttributePoints �� ������? 
		if (IPlayerInterface::Execute_GetAttributePoints(GetAvatarActor()) > 0)
		{
			// �������� ó��
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
		// AttributePoints�� ����� ��� ����.
		IPlayerInterface::Execute_AddToAttributePoints(GetAvatarActor(), -1);
	}
}

// �ʿ� ������ �Ѱ�����, Grant���� �ʴ� Ability�� Ȱ��ȭ ���ִ� �Լ�.
void UDreamAbilitySystemComponent::UpdateAbilityStatuses(int32 Level)
{
	UAbilityInfo* AbilityInfo = UDreamAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	for (const FDreamAbilityInfo Info : AbilityInfo->AbilityInformation)	// AbilityInformation�� �ִ� ��� GA�� ��ȸ.
	{
		if (!Info.AbilityTag.IsValid()) continue;	// AbilityTag�� ���� ��� -> �н�
		if (Level < Info.LevelRequirement) continue;	// �ʿ� ������ �ȵǴ� ��� -> �н�
		// AbilitySpec�� ���� ���? -> AbilitySpec ����.
		if (GetSpecFromAbilityTag(Info.AbilityTag) == nullptr)
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Info.Ability, 1);
			// �ʿ䷹���� ���� -> Status : Eligible
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
	// ������ �������� �ɷ�ġ ����
	FGameplayEventData Payload;
	Payload.EventTag = AttributeTag;
	Payload.EventMagnitude = Magnitude;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, Payload);

}

void UDreamAbilitySystemComponent::RemoveItem(const FGameplayTag& AttributeTag, const FGameplayTag& StatusTag, int32 Magnitude)
{
	// �������� �������� �ƴ϶�� ����.
	if (StatusTag == FDreamGameplayTags::Get().Item_Status_Normal)	return;

	FGameplayEventData PrevPayload;
	PrevPayload.EventTag = AttributeTag;
	PrevPayload.EventMagnitude = -Magnitude; // - ��ȣ�� �ٿ� ������ ��, ��ġ��ŭ ���� �� ����.
	// ���� ���̴� �ɷ�ġ�� ����(����).
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
			// Eligible �±׸� ����� Unlocked �±� �߰� -> Tag�� ���� Status ����
			AbilitySpec->DynamicAbilityTags.RemoveTag(GameplayTags.Abilities_Status_Eligible);
			AbilitySpec->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Unlocked);
			Status = GameplayTags.Abilities_Status_Unlocked;	// Status Update

		}
		if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Equipped) || Status.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
		{
			AbilitySpec->Level += 1;
		}
		ClientUpdateAbilityStatus(AbilityTag, Status, AbilitySpec->Level);	// Updatg�� Status�� Client�� ������Ʈ
		MarkAbilitySpecDirty(*AbilitySpec);	// Force AbilitySpec To be Replicated 
	}
}

// 2 SkillRowBoxPressed�� ���� ��ų�� �����ϴ� ���� ����
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

			// �̹� ������ ��ų�� �ִ� ���.
			if (!SlotIsEmpty(Slot))  // There is an ability in this slot already. Deactivate and clear its slot.
			{
				// �ش� ���Կ� �ִ� Ability
				FGameplayAbilitySpec* SpecWithSlot = GetSpecWithSlot(Slot);
				if (SpecWithSlot)
				{
					// �����Ϸ��� ��ų = �������� ��ų�� ���. => early return.
					// is that ability the same as this ability? If so, we can return early.
					if (AbilityTag.MatchesTagExact(GetAbilityTagFromSpec(*SpecWithSlot)))
					{
						ClientEquipAbility(AbilityTag, GameplayTags.Abilities_Status_Equipped, Slot, PrevSlot);
						return;
					}

					// �̹� �������� PassiveAbility �� Deactivate�ϱ� ���� ��������Ʈ�� Broadcast
					if (IsPassiveAbility(*SpecWithSlot))
					{
						// ������ Ȱ��ȭ�� PassiveAbility�� DeActivate(false)
						// ActivatePassiveEffect(Tag, false)�� �ѱ�.
						MulticastActivatePassiveEffect(GetAbilityTagFromSpec(*SpecWithSlot), false);
						DeactivatePassiveAbility.Broadcast(GetAbilityTagFromSpec(*SpecWithSlot));
					}
					ClearSlot(SpecWithSlot);
				}
			}

			// ���ο� Ability�� ����(Slot�� �������� �ʾҴٸ�)
			if (!AbilityHasAnySlot(*AbilitySpec)) // Ability doesn't yet have a slot (it's not active)
			{
				if (IsPassiveAbility(*AbilitySpec))
				{
					TryActivateAbility(AbilitySpec->Handle);
					// ���ο� Passive Ability�� Ȱ��ȭ(true)
					// ActivatePassiveEffect(Tag, true)�� �ѱ�.
					MulticastActivatePassiveEffect(AbilityTag, true);
				}
				AbilitySpec->DynamicAbilityTags.RemoveTag(GetStatusFromSpec(*AbilitySpec));
				AbilitySpec->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Equipped);
			}
			AssignSlotToAbility(*AbilitySpec, Slot);
			MarkAbilitySpecDirty(*AbilitySpec);

			/*
			// Remove this InputTag (slot) from any Ability that has it.
			// ������ ���Կ� Ability�� �̹� �����Ѵٸ�, Ability�� �����ְ�
			ClearAbilitiesOfSlot(Slot);
			// Clear this ability's slot, just in case, it's a different slot
			// ������ �ִ� Ability�� ���� InputTag�� ����.
			ClearSlot(AbilitySpec);
			// Now, assign this ability to this slot
			// ���� ������ Ability�� InputTag(Slot)��	�߰�
			AbilitySpec->DynamicAbilityTags.AddTag(Slot);
			if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
			{
				AbilitySpec->DynamicAbilityTags.RemoveTag(GameplayTags.Abilities_Status_Unlocked);
				AbilitySpec->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Equipped);
			}
			MarkAbilitySpecDirty(*AbilitySpec);
			*/
		}
		// 3 Cached�� ������ ���ֱ� ���� Client�� �˸�.
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
	// Ȱ��ȭ�� Ability�� �ش� Slot�� ������ �ִ� Ability�� Clear
	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (AbilityHasSlot(&Spec, Slot))
		{
			ClearSlot(&Spec);
		}
	}
}

// AbilitySpec�� �ش� Slot�� ������ �ִ��� �Ǻ��ϴ� �Լ�
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

	// ó���� Ŭ���̾�Ʈ������ false�̱� ������ ó���� ����.
	if (!bStartupAbilitiesGiven)
	{
		// true�� �ؾ� OnInitializeStartupAbilities() �Լ� ���� ����.
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
