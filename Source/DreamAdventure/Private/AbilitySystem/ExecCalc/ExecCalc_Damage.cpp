// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"

#include "DreamAbilityTypes.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/DreamAttributeSet.h"
#include "AbilitySystem/DreamAbilitySystemLibrary.h"
#include "DreamGameplayTags.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Interaction/CombatInterface.h"

#include "Camera/CameraShakeSourceActor.h"
#include "Kismet/GameplayStatics.h"

// AttributeDef / CaptureObject / SnapShot ���� �����ϰ� �����ϴ� Boilerplate
struct DreamDamageStatics
{
	//#define DECLARE_ATTRIBUTE_CAPTUREDEF(P) \
	//FProperty* P##Property; \
	//FGameplayEffectAttributeCaptureDefinition P##Def; \

	// Attribute�� ������ Property �� FGameplayEffectAttributeCaptureDefinition�� �������ִ� ��ũ��
	DECLARE_ATTRIBUTE_CAPTUREDEF(Strength);	// Attribute -> AttributeDef �� ������ִ� ��ũ��
	DECLARE_ATTRIBUTE_CAPTUREDEF(Intelligence);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitDamage);

	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(MagicalResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(FireResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(LightningResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(IceResistance);

	DreamDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UDreamAttributeSet, Strength, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UDreamAttributeSet, Intelligence, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UDreamAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UDreamAttributeSet, ArmorPenetration, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UDreamAttributeSet, CriticalHitChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UDreamAttributeSet, CriticalHitDamage, Source, false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(UDreamAttributeSet, PhysicalResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UDreamAttributeSet, MagicalResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UDreamAttributeSet, FireResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UDreamAttributeSet, LightningResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UDreamAttributeSet, IceResistance, Target, false);


	
		// �Ʒ��� �ڵ�, ������ �����ϴ� ��ũ��
		// UDreamAttributeSet -> StaticClass
		// VigorDef.AttributeToCapture = UDreamAttributeSet::GetVigorAttribute();	// Capture �� Attribute
		// VigorDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
		// VigorDef.bSnapshot = false;
	}
};

// static ����ü�� �����ϴ� �Լ�.
static const DreamDamageStatics& DamageStatics()
{
	static DreamDamageStatics DStatics;
	return DStatics;
}

UExecCalc_Damage::UExecCalc_Damage()
{
	/** Attributes to capture that are relevant to the calculation */
	// ��꿡 ���õ� Atttibute�� Capture�ϱ� ���� RelevantAttributesToCapture��� �迭�� �߰�.
	RelevantAttributesToCapture.Add(DamageStatics().StrengthDef);
	RelevantAttributesToCapture.Add(DamageStatics().IntelligenceDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArmorPenetrationDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitDamageDef);
	RelevantAttributesToCapture.Add(DamageStatics().PhysicalResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().MagicalResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().FireResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().LightningResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().IceResistanceDef);
}

void UExecCalc_Damage::DetermineDebuff(const FGameplayEffectCustomExecutionParameters& ExecutionParams, const FGameplayEffectSpec& Spec, FAggregatorEvaluateParameters EvaluationParameters, const TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition>& InTagsToDefs) const
{
	const FDreamGameplayTags& GameplayTags = FDreamGameplayTags::Get();

	for (TTuple<FGameplayTag, FGameplayTag> Pair : GameplayTags.DamageTypesToDebuffs)
	{
		const FGameplayTag& DamageType = Pair.Key;
		const FGameplayTag& DebuffType = Pair.Value;

		// GameplayEffect���� Pair.Key(=Damage.Type)�� �ش��ϴ� �����͸� ã�Ƽ� ����( ������ -1�� ����)
		const float TypeDamage = Spec.GetSetByCallerMagnitude(Pair.Key, false, -1.f);
		if (TypeDamage > -0.5f) // .5 padding for floating point imprecision
		{
			const float SourceDebuffChance = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Chance, false, -1.f);

			float TargetDebuffResistance = 0.f;
			const FGameplayTag& ResistanceTag = GameplayTags.DamageTypesToResistances[DamageType];
			ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(InTagsToDefs[ResistanceTag], EvaluationParameters, TargetDebuffResistance);
			TargetDebuffResistance = FMath::Max<float>(TargetDebuffResistance, 0.f);
			const float EffectiveDebuffChance = SourceDebuffChance * (100 - TargetDebuffResistance) / 100.f;
			const bool bDebuff = FMath::RandRange(1, 100) < EffectiveDebuffChance;
			if (bDebuff)
			{
				FGameplayEffectContextHandle ContextHandle = Spec.GetContext();

				UDreamAbilitySystemLibrary::SetIsSuccessfulDebuff(ContextHandle, true);
				UDreamAbilitySystemLibrary::SetDamageType(ContextHandle, DamageType);

				// UDreamAbilitySystemLibrary::ApplyDamageEffect ���� SetByCallerMagnitude �� ������ ���� ������
				const float DebuffDamage = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Damage, false, -1.f);
				const float DebuffDuration = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Duration, false, -1.f);
				const float DebuffFrequency = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Frequency, false, -1.f);

				UDreamAbilitySystemLibrary::SetDebuffDamage(ContextHandle, DebuffDamage);
				UDreamAbilitySystemLibrary::SetDebuffDuration(ContextHandle, DebuffDuration);
				UDreamAbilitySystemLibrary::SetDebuffFrequency(ContextHandle, DebuffFrequency);


			}

		}
	}
}

// GE�� ��������
void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	// Tag�� �ش��ϴ� ���� �����ϱ� ���� TMap
	TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToCaptureDefs;

	const FDreamGameplayTags& Tags = FDreamGameplayTags::Get();

	TagsToCaptureDefs.Add(Tags.Attributes_Primary_Strength, DamageStatics().StrengthDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Primary_Intelligence, DamageStatics().IntelligenceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_Armor, DamageStatics().ArmorDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_ArmorPenetration, DamageStatics().ArmorPenetrationDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitChance, DamageStatics().CriticalHitChanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitDamage, DamageStatics().CriticalHitDamageDef);

	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Physical, DamageStatics().PhysicalResistanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Magical, DamageStatics().MagicalResistanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Fire, DamageStatics().FireResistanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Lightning, DamageStatics().LightningResistanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Ice, DamageStatics().IceResistanceDef);

	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	int32 SourcePlayerLevel = 1;
	if (SourceAvatar->Implements<UCombatInterface>())
	{
		SourcePlayerLevel = ICombatInterface::Execute_GetPlayerLevel(SourceAvatar);
	}
	int32 TargetPlayerLevel = 1;
	if (TargetAvatar->Implements<UCombatInterface>())
	{
		TargetPlayerLevel = ICombatInterface::Execute_GetPlayerLevel(TargetAvatar);
	}

	const FGameplayEffectSpec Spec = ExecutionParams.GetOwningSpec();
	FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();
	// 
	// Captured Attribute�� ������( EGameplayEffectAttributeCaptureSource�� Source / Target ���� �з�)
	/** Captured Source Tags on GameplayEffectSpec creation */
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// Debuff
	DetermineDebuff(ExecutionParams, Spec, EvaluationParameters, TagsToCaptureDefs);

	// Damage�� DreamDamageGameplayAbility���� AssignTagSetByCallerMagnitdue �� ���� Tag�� ������ ���� ������.
	// �� ��ų - Damage �±׿� ������ Value�� ������.
	float Damage = 0.f;
	for (const TTuple<FGameplayTag, FGameplayTag>& Pair : FDreamGameplayTags::Get().DamageTypesToResistances)
	{
		// DreamPrjectileSpell(GameplayAbility Ŭ����) ���� AssignTagSetByCallerMagnitude�� ���� Tag�� ������ ���� ������.
		const FGameplayTag DamageTag = Pair.Key;
		const FGameplayTag ResistanceTag = Pair.Value;
		
		checkf(TagsToCaptureDefs.Contains(ResistanceTag), TEXT("TagsToCaptureDefs doesn't contain Tag: [%s] in ExecCalc_Damage"), *ResistanceTag.ToString());
		// TagsToCaptureDefs ���� �ش�(Resistance) �±׿� �ش��ϴ� Def(��)�� ������.
		const FGameplayEffectAttributeCaptureDefinition CaptureDef = TagsToCaptureDefs[ResistanceTag];

		// false => DamageType�� ���� �����Ǿ� ���� ������ ���X
		float DamageTypeValue = Spec.GetSetByCallerMagnitude(Pair.Key, false);
		
		if (DamageTypeValue <= 0.f)
		{
			continue;
		}

		float Resistance = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureDef, EvaluationParameters, Resistance);
		Resistance = FMath::Max<float>(Resistance, 0.f);

		DamageTypeValue *= (100.f - Resistance) / 100.f;

		if (UDreamAbilitySystemLibrary::IsRadialDamage(EffectContextHandle))
		{
			// Procedure To Take Radial Damage
			// 1. override TakeDamage in AuraCharacterBase. *
			// 2. create delegate OnDamageDelegate, broadcast damage received in TakeDamage *
			// 3. Bind lambda to OnDamageDelegate on the Victim here. *
			// 4. Call UGameplayStatics::ApplyRadialDamageWithFalloff to cause damage (this will result in TakeDamage being called
			//		on the Victim, which will then broadcast OnDamageDelegate)
			// 5. In Lambda, set DamageTypeValue to the damage received from the broadcast *
			
			
			if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(TargetAvatar))
			{
				// 3. Bind lambda to OnDamageDelegate on the Victim here. *
				CombatInterface->GetOnDamageSignature().AddLambda([&](float DamageAmount) // [&]: ĸ���� �����͸� Reference���·� ������.(-> ���� �����Ǹ� �ݿ���)
					{
						// 5. In Lambda, set DamageTypeValue to the damage received from the broadcast *
						DamageTypeValue = DamageAmount;
					});
			}

			// 4. Call UGameplayStatics::ApplyRadialDamageWithFalloff to cause damage (this will result in TakeDamage being called
			//		on the Victim, which will then broadcast OnDamageDelegate)
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				TargetAvatar,	// WorldContextObject
				DamageTypeValue,	// BaseDamage
				0.f,				// Mimimum Damage
				UDreamAbilitySystemLibrary::GetRadialDamageOrigin(EffectContextHandle),		// Damage Origin
				UDreamAbilitySystemLibrary::GetRadialDamageInnerRadius(EffectContextHandle),	// InnerRadius
				UDreamAbilitySystemLibrary::GetRadialDamageOuterRadius(EffectContextHandle),	// OuterRadius
				1.f,				// Fallof
				UDamageType::StaticClass(),	// DamageTypeClass
				TArray<AActor*>(),	// Ignore Actors
				SourceAvatar,	// Damage Causer
				nullptr			// Controller	
			);
		}
		Damage += DamageTypeValue;
	}

	float PhysicalDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().StrengthDef, EvaluationParameters, PhysicalDamage);
	PhysicalDamage = FMath::Max<float>(PhysicalDamage, 0.f);
	
	float MagicalDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().IntelligenceDef, EvaluationParameters, MagicalDamage);
	MagicalDamage = FMath::Max<float>(MagicalDamage, 0.f);

	float TotalDamage = Damage + PhysicalDamage + MagicalDamage;

	float CriticalHitChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitChanceDef, EvaluationParameters, CriticalHitChance);
	CriticalHitChance = FMath::Max<float>(CriticalHitChance, 0.f);

	const bool bCriticalHit = FMath::RandRange(0, 100) < CriticalHitChance;

	UDreamAbilitySystemLibrary::SetIsCriticalHit(EffectContextHandle, bCriticalHit);

	float CriticalHitDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitDamageDef, EvaluationParameters, CriticalHitDamage);
	CriticalHitDamage = FMath::Max<float>(0.f, CriticalHitDamage);

	if (bCriticalHit)
	{
		TotalDamage = 2* TotalDamage + CriticalHitDamage;
	}
	else
	{
		TotalDamage = TotalDamage;
	}
	
	float Armor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluationParameters, Armor);
	Armor = FMath::Max<float>(Armor, 0.f);

	const UCharacterClassInfo* CharacterClassInfo = UDreamAbilitySystemLibrary::GetCharacterClassInfo(SourceAvatar);
	// Ŀ�� ���̺��� ���� �̸��� �ش��ϴ� ���� ������.
	//const FRealCurve* ArmorPenetrationCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("ArmorPenetration"), FString());
	//const float ArmorPenetrationCoefficient = ArmorPenetrationCurve->Eval(SourceCombatInterface->GetPlayerLevel());

	float RealDamage = TotalDamage - Armor;

	const FGameplayModifierEvaluatedData EvaluatedData(UDreamAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, RealDamage);
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}
