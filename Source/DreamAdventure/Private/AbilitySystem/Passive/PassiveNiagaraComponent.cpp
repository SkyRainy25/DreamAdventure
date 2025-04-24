// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Passive/PassiveNiagaraComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/DreamAbilitySystemComponent.h"
#include "Interaction/CombatInterface.h"
#include "DreamGameplayTags.h"

UPassiveNiagaraComponent::UPassiveNiagaraComponent()
{
	bAutoActivate = false;
}

void UPassiveNiagaraComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UDreamAbilitySystemComponent* DreamASC = Cast<UDreamAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner())))
	{
		DreamASC->ActivatePassiveEffect.AddUObject(this, &UPassiveNiagaraComponent::OnPassiveActivate);
		ActivateIfEquipped(DreamASC);
	}
	else if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetOwner()))
	{
		CombatInterface->GetOnASCRegisteredDelegate().AddLambda([this](UAbilitySystemComponent* ASC)
			{
				if (UDreamAbilitySystemComponent* DreamASC = Cast<UDreamAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner())))
				{
					DreamASC->ActivatePassiveEffect.AddUObject(this, &UPassiveNiagaraComponent::OnPassiveActivate);
					ActivateIfEquipped(DreamASC);
				}
			});
	}
}

void UPassiveNiagaraComponent::ActivateIfEquipped(UDreamAbilitySystemComponent* DreamASC)
{
	const bool bStartupAbilitiesGiven = DreamASC->bStartupAbilitiesGiven;
	if (bStartupAbilitiesGiven)
	{
		if (DreamASC->GetStatusFromAbilityTag(PassiveSkillTag) == FDreamGameplayTags::Get().Abilities_Status_Equipped)
		{
			Activate();
		}
	}
}

void UPassiveNiagaraComponent::OnPassiveActivate(const FGameplayTag& AbilityTag, bool bActivate)
{
	// ActivePassiveEffect 델리게이트를 통해, 넘겨받은 boolean으로 활성화/비활성화 
	if (AbilityTag.MatchesTagExact(PassiveSkillTag))
	{
		if (bActivate && !IsActive())
		{
			Activate();
		}
		else
		{
			Deactivate();
		}
	}
}

