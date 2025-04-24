// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/DreamAbilitySystemGlobals.h"

#include "DreamAbilityTypes.h"

FGameplayEffectContext* UDreamAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FDreamGameplayEffectContext();
}
