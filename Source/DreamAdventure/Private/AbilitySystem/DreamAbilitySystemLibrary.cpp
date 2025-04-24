// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/DreamAbilitySystemLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DreamGameplayTags.h"
#include "UI/WidgetController/DreamWidgetController.h"
#include "Player/DreamPlayerState.h"
#include "UI/HUD/DreamHUD.h"
#include "Interaction/CombatInterface.h"

#include "DreamAbilityTypes.h"
#include "Game/DreamGameModeBase.h"
#include "Interaction/PlayerInterface.h"
#include "Game/LoadScreenSaveGame.h"

bool UDreamAbilitySystemLibrary::MakeWidgetControllerParams(const UObject* WorldContextObject, FWidgetControllerParams& OutWCParams, ADreamHUD*& OutDreamHUD)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		// Pointer + Reference�̱� ������, �ٱ��� �����Ϳ� ��ȭ�� �ݿ�.
		// InputParam ���� �־��� DreamHUD(OutDreamHUD)�� �����͸� �ٲ�.
		OutDreamHUD = Cast<ADreamHUD>(PC->GetHUD());
		if (OutDreamHUD)
		{
			ADreamPlayerState* PS = PC->GetPlayerState<ADreamPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();

			OutWCParams.AttributeSet = AS;
			OutWCParams.AbilitySystemComponent = ASC;
			OutWCParams.PlayerState = PS;
			OutWCParams.PlayerController = PC;
			return true;
		}
	}
	return false;
}

UOverlayWidgetController* UDreamAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	ADreamHUD* DreamHUD = nullptr;
	if (MakeWidgetControllerParams(WorldContextObject, WCParams, DreamHUD))
	{
		return DreamHUD->GetOverlayWidgetController(WCParams);
	}
	return nullptr;
}

UAttributeMenuWidgetController* UDreamAbilitySystemLibrary::GetAttributeMenuWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	ADreamHUD* DreamHUD = nullptr;
	// DreamHUD�� MakeWidgetControllerParmas�� ���� DreamHUD�� �����Ͱ� ä���� -> *& (Pointer + Reference)
	if (MakeWidgetControllerParams(WorldContextObject, WCParams, DreamHUD))
	{
		return DreamHUD->GetAttributeMenuWidgetController(WCParams);
	}
	return nullptr;
}

USkillMenuWidgetController* UDreamAbilitySystemLibrary::GetSkillMenuWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	ADreamHUD* DreamHUD = nullptr;
	if (MakeWidgetControllerParams(WorldContextObject, WCParams, DreamHUD))
	{
		return DreamHUD->GetSkillMenuWidgetController(WCParams);
	}
	return nullptr;
}

UInventoryWidgetController* UDreamAbilitySystemLibrary::GetInventoryWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	ADreamHUD* DreamHUD = nullptr;
	if (MakeWidgetControllerParams(WorldContextObject, WCParams, DreamHUD))
	{
		return DreamHUD->GetInventoryWidgetController(WCParams);
	}
	return nullptr;
}

UStoreWidgetController* UDreamAbilitySystemLibrary::GetStoreWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	ADreamHUD* DreamHUD = nullptr;
	if (MakeWidgetControllerParams(WorldContextObject, WCParams, DreamHUD))
	{
		return DreamHUD->GetStoreWidgetController(WCParams);
	}
	return nullptr;
}

void UDreamAbilitySystemLibrary::InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC)
{

	AActor* AvatarActor = ASC->GetAvatarActor();

	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	FCharacterClassDefaultInfo ClassDefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);

	FGameplayEffectContextHandle PrimaryAttributesContextHandle = ASC->MakeEffectContext();
	PrimaryAttributesContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle PrimaryAttributesSpecHandle = ASC->MakeOutgoingSpec(ClassDefaultInfo.PrimaryAttributes, Level, PrimaryAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*PrimaryAttributesSpecHandle.Data.Get());

	FGameplayEffectContextHandle SecondaryAttributesContextHandle = ASC->MakeEffectContext();
	SecondaryAttributesContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle SecondaryAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->SecondaryAttributes, Level, SecondaryAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*SecondaryAttributesSpecHandle.Data.Get());

	FGameplayEffectContextHandle VitalAttributesContextHandle = ASC->MakeEffectContext();
	VitalAttributesContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle VitalAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->VitalAttributes, Level, VitalAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*VitalAttributesSpecHandle.Data.Get());
}

void UDreamAbilitySystemLibrary::InitializeDefaultAttributesFromSaveData(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ULoadScreenSaveGame* SaveGame)
{
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr) return;

	const FDreamGameplayTags& GameplayTags = FDreamGameplayTags::Get();

	const AActor* SourceAvatarActor = ASC->GetAvatarActor();

	FGameplayEffectContextHandle EffectContexthandle = ASC->MakeEffectContext();
	EffectContexthandle.AddSourceObject(SourceAvatarActor);

	// ����� PrimaryAttributes�� ����(AssginSetByCallerMagnitude�� ����)
	const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->PrimaryAttributes_SetByCaller, 1.f, EffectContexthandle);

	// SaveData�� ����� ���� ������.
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Strength, SaveGame->Strength);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Intelligence, SaveGame->Intelligence);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Resilience, SaveGame->Resilience);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Vigor, SaveGame->Vigor);

	// GameplayEffect ����
	ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);

	// SecondaryAttribute ����
	FGameplayEffectContextHandle SecondaryAttributesContextHandle = ASC->MakeEffectContext();
	SecondaryAttributesContextHandle.AddSourceObject(SourceAvatarActor);
	const FGameplayEffectSpecHandle SecondaryAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->SecondaryAttributes_Infinite, 1.f, SecondaryAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*SecondaryAttributesSpecHandle.Data.Get());

	// Vital Attributes ����
	FGameplayEffectContextHandle VitalAttributesContextHandle = ASC->MakeEffectContext();
	VitalAttributesContextHandle.AddSourceObject(SourceAvatarActor);
	const FGameplayEffectSpecHandle VitalAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->VitalAttributes, 1.f, VitalAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*VitalAttributesSpecHandle.Data.Get());
}

void UDreamAbilitySystemLibrary::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass)
{
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr)	return;
	for (TSubclassOf<UGameplayAbility> AbilityClass : CharacterClassInfo->CommonAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		ASC->GiveAbility(AbilitySpec);
	}
	const FCharacterClassDefaultInfo& DefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
	for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultInfo.StartupAbilities)
	{
		if (ASC->GetAvatarActor()->Implements<UCombatInterface>())
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, ICombatInterface::Execute_GetPlayerLevel(ASC->GetAvatarActor()));
			ASC->GiveAbility(AbilitySpec);
		}
	}
}

UCharacterClassInfo* UDreamAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	const ADreamGameModeBase* DreamGameMode = Cast<ADreamGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (DreamGameMode == nullptr) return nullptr;
	return DreamGameMode->CharacterClassInfo;
}

UAbilityInfo* UDreamAbilitySystemLibrary::GetAbilityInfo(const UObject* WorldContextObject)
{
	const ADreamGameModeBase* DreamGameMode = Cast<ADreamGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (DreamGameMode == nullptr) return nullptr;
	return DreamGameMode->AbilityInfo;
}

ULootTiers* UDreamAbilitySystemLibrary::GetLootTiers(const UObject* WorldContextObject)
{
	const ADreamGameModeBase* DreamGameMode = Cast<ADreamGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (DreamGameMode == nullptr) return nullptr;
	return DreamGameMode->LootTiers;
}

UItemInfo* UDreamAbilitySystemLibrary::GetItemInfo(const UObject* WorldContextObject)
{
	const ADreamGameModeBase* DreamGameMode = Cast<ADreamGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (DreamGameMode == nullptr)	return nullptr;
	return DreamGameMode->ItemInfo;
}

bool UDreamAbilitySystemLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FDreamGameplayEffectContext* DreamEffectContext = static_cast<const FDreamGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return DreamEffectContext->IsBlockedHit();
	}
	return false;
}

bool UDreamAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FDreamGameplayEffectContext* DreamEffectContext = static_cast<const FDreamGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return DreamEffectContext->IsCriticalHit();
	}
	return false;
}

bool UDreamAbilitySystemLibrary::IsSuccessfulDebuff(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FDreamGameplayEffectContext* DreamEffectContext = static_cast<const FDreamGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return DreamEffectContext->IsSuccessfulDebuff();
	}
	return false;
}

float UDreamAbilitySystemLibrary::GetDebuffDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FDreamGameplayEffectContext* DreamEffectContext = static_cast<const FDreamGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return DreamEffectContext->GetDebuffDamage();
	}
	return 0.f;
}

float UDreamAbilitySystemLibrary::GetDebuffDuration(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FDreamGameplayEffectContext* DreamEffectContext = static_cast<const FDreamGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return DreamEffectContext->GetDebuffDuration();
	}
	return 0.f;
}

float UDreamAbilitySystemLibrary::GetDebuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FDreamGameplayEffectContext* DreamEffectContext = static_cast<const FDreamGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return DreamEffectContext->GetDebuffFrequency();
	}
	return 0.f;
}

FGameplayTag UDreamAbilitySystemLibrary::GetDamageType(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FDreamGameplayEffectContext* DreamEffectContext = static_cast<const FDreamGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		if (DreamEffectContext->GetDamageType().IsValid())
		{
			return *DreamEffectContext->GetDamageType();
		}
	}
	return FGameplayTag();
}

FVector UDreamAbilitySystemLibrary::GetDeathImpulse(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FDreamGameplayEffectContext* DreamEffectContext = static_cast<const FDreamGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return DreamEffectContext->GetDeathImpulse();
	}
	return FVector::ZeroVector;
}

FVector UDreamAbilitySystemLibrary::GetKnockbackForce(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FDreamGameplayEffectContext* DreamEffectContext = static_cast<const FDreamGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return DreamEffectContext->GetKnockbackForce();
	}
	return FVector::ZeroVector;
}

bool UDreamAbilitySystemLibrary::IsRadialDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FDreamGameplayEffectContext* DreamEffectContext = static_cast<const FDreamGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return DreamEffectContext->IsRadialDamage();
	}
	return false;
}

float UDreamAbilitySystemLibrary::GetRadialDamageInnerRadius(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FDreamGameplayEffectContext* DreamEffectContext = static_cast<const FDreamGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return DreamEffectContext->GetRadialDamageInnerRadius();
	}
	return 0.f;
}

float UDreamAbilitySystemLibrary::GetRadialDamageOuterRadius(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FDreamGameplayEffectContext* DreamEffectContext = static_cast<const FDreamGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return DreamEffectContext->GetRadialDamageOuterRadius();
	}
	return 0.f;
}

FVector UDreamAbilitySystemLibrary::GetRadialDamageOrigin(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FDreamGameplayEffectContext* DreamEffectContext = static_cast<const FDreamGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return DreamEffectContext->GetRadialDamageOrigin();
	}
	return FVector::ZeroVector;
}

void UDreamAbilitySystemLibrary::SetIsBlockedHit(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit)
{
	if (FDreamGameplayEffectContext* DreamEffectContext = static_cast<FDreamGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		DreamEffectContext->SetIsCriticalHit(bInIsBlockedHit);
	}
}

void UDreamAbilitySystemLibrary::SetIsCriticalHit(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, bool bInIsCriticalHit)
{
	if (FDreamGameplayEffectContext* DreamEffectContext = static_cast<FDreamGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		DreamEffectContext->SetIsCriticalHit(bInIsCriticalHit);
	}
}

void UDreamAbilitySystemLibrary::SetIsSuccessfulDebuff(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, bool bInSuccessfulDebuff)
{
	if (FDreamGameplayEffectContext* DreamEffectContext = static_cast<FDreamGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		DreamEffectContext->SetIsSuccessfulDebuff(bInSuccessfulDebuff);
	}
}

void UDreamAbilitySystemLibrary::SetDebuffDamage(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, float InDamage)
{
	if (FDreamGameplayEffectContext* DreamEffectContext = static_cast<FDreamGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		DreamEffectContext->SetDebuffDamage(InDamage);
	}
}

void UDreamAbilitySystemLibrary::SetDebuffDuration(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, float InDuration)
{
	if (FDreamGameplayEffectContext* DreamEffectContext = static_cast<FDreamGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		DreamEffectContext->SetDebuffDuration(InDuration);
	}
}

void UDreamAbilitySystemLibrary::SetDebuffFrequency(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, float InFrequency)
{
	if (FDreamGameplayEffectContext* DreamEffectContext = static_cast<FDreamGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		DreamEffectContext->SetDebuffFrequency(InFrequency);
	}
}

void UDreamAbilitySystemLibrary::SetDamageType(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, const FGameplayTag& InDamageType)
{
	if (FDreamGameplayEffectContext* DreamEffectContext = static_cast<FDreamGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		const TSharedPtr<FGameplayTag> DamageType = MakeShared<FGameplayTag>(InDamageType);
		DreamEffectContext->SetDamageType(DamageType);
	}
}

void UDreamAbilitySystemLibrary::SetDeathImpulse(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, const FVector& InImpulse)
{
	if (FDreamGameplayEffectContext* DreamEffectContext = static_cast<FDreamGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		DreamEffectContext->SetDeathImpulse(InImpulse);
	}
}

void UDreamAbilitySystemLibrary::SetKnockbackForce(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, const FVector& InForce)
{
	if (FDreamGameplayEffectContext* DreamEffectContext = static_cast<FDreamGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		DreamEffectContext->SetKnockbackForce(InForce);
	}
}

void UDreamAbilitySystemLibrary::SetIsRadialDamage(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, bool bInIsRadialDamage)
{
	if (FDreamGameplayEffectContext* DreamEffectContext = static_cast<FDreamGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		DreamEffectContext->SetIsRadialDamage(bInIsRadialDamage);
	}
}

void UDreamAbilitySystemLibrary::SetRadialDamageInnerRadius(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, float InInnerRadius)
{
	if (FDreamGameplayEffectContext* DreamEffectContext = static_cast<FDreamGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		DreamEffectContext->SetRadialDamageInnerRadius(InInnerRadius);
	}
}

void UDreamAbilitySystemLibrary::SetRadialDamageOuterRadius(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, float InOuterRadius)
{
	if (FDreamGameplayEffectContext* DreamEffectContext = static_cast<FDreamGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		DreamEffectContext->SetRadialDamageOuterRadius(InOuterRadius);
	}
}

void UDreamAbilitySystemLibrary::SetRadialDamageOrigin(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, const FVector& InOrigin)
{
	if (FDreamGameplayEffectContext* DreamEffectContext = static_cast<FDreamGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		DreamEffectContext->SetRadialDamageOrigin(InOrigin);
	}
}

void UDreamAbilitySystemLibrary::GetLivePlayersWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& SphereOrigin)
{
	FCollisionQueryParams SphereParams;
	SphereParams.AddIgnoredActors(ActorsToIgnore);

	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		TArray<FOverlapResult> Overlaps;	
		World->OverlapMultiByObjectType(
			Overlaps,
			SphereOrigin,
			FQuat::Identity,
			FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects),
			FCollisionShape::MakeSphere(Radius),
			SphereParams);

		for (FOverlapResult& Overlap : Overlaps)
		{
			// CombatInterface�� ���� ��������? => BaseCharacter Ŭ�������� + �׾����� �Ǻ�
			if (Overlap.GetActor()->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsDead(Overlap.GetActor()))
			{
				OutOverlappingActors.AddUnique(ICombatInterface::Execute_GetAvatar(Overlap.GetActor()));
			}
		}	
	}
}

void UDreamAbilitySystemLibrary::GetClosestTargets(int32 MaxTargets, const TArray<AActor*>& Actors, TArray<AActor*>& OutClosestTargets, const FVector& Origin)
{
	// Radius(�ݰ�)�� �������� ������ �� < �ִ� �߰��� �� �ִ� ��
	if (Actors.Num() <= MaxTargets)
	{
		OutClosestTargets = Actors;
		return;
	}
	TArray<AActor*> ActorsToCheck = Actors;
	int32 NumTargetsFound = 0;

	while (NumTargetsFound < MaxTargets)
	{
		// �˻��ؾ��� Actor�� ���ٸ� �н�
		if (ActorsToCheck.Num() == 0) break;
		double ClosestDistance = TNumericLimits<double>::Max();
		AActor* ClosestActor;	// ���� ����� ���͸� ������ ����.
		for (AActor* PotentialTarget : ActorsToCheck)	// �Ѱܹ��� ������ �迭�� ��ȸ�ϸ�
		{
			// ���� ū������ �ʱ� ���� ����
			const double Distance = (PotentialTarget->GetActorLocation() - Origin).Length();
			if (Distance < ClosestDistance)	// ���� ����� ���� �߰ߵǸ�
			{
				// �ֽ�ȭ �� ����
				ClosestDistance = Distance;
				ClosestActor = PotentialTarget;
			}
		}
		// ���� ����� ���͸� ã������, ActorsToCheck(�˻���)���� ���� 
		// + OutParams�� ������ OutClosestTargets �迭�� �߰�(AddUnique)
		// => NumTargetsFound�� While���� ������� ���� ������, �� ���� �ݺ�(���� ����� ���� �� �Ÿ��� ��� ã�� ����);
		ActorsToCheck.Remove(ClosestActor);
		OutClosestTargets.AddUnique(ClosestActor);
		++NumTargetsFound;
	}
}

bool UDreamAbilitySystemLibrary::IsNotFriend(AActor* FirstActor, AActor* SecondActor)
{
	const bool bBothArePlayers = FirstActor->ActorHasTag(FName("Player")) && SecondActor->ActorHasTag(FName("Player"));
	const bool bBothAreEnemies = FirstActor->ActorHasTag(FName("Enemy")) && SecondActor->ActorHasTag(FName("Enemy"));
	const bool bFriends = bBothArePlayers || bBothAreEnemies;
	return !bFriends;
}

FGameplayEffectContextHandle UDreamAbilitySystemLibrary::ApplyDamageEffect(const FDamageEffectParams& DamageEffectParams)
{
	const FDreamGameplayTags& GameplayTags = FDreamGameplayTags::Get();
	const AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor();

	FGameplayEffectContextHandle EffectContextHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeEffectContext();
	EffectContextHandle.AddSourceObject(SourceAvatarActor);
	SetDeathImpulse(EffectContextHandle, DamageEffectParams.DeathImpulse);
	SetKnockbackForce(EffectContextHandle, DamageEffectParams.KnockbackForce);

	SetIsRadialDamage(EffectContextHandle, DamageEffectParams.bIsRadialDamage);
	SetRadialDamageInnerRadius(EffectContextHandle, DamageEffectParams.RadialDamageInnerRadius);
	SetRadialDamageOuterRadius(EffectContextHandle, DamageEffectParams.RadialDamageOuterRadius);
	SetRadialDamageOrigin(EffectContextHandle, DamageEffectParams.RadialDamageOrigin);

	const FGameplayEffectSpecHandle SpecHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeOutgoingSpec(
		DamageEffectParams.DamageGameplayEffectClass,
		DamageEffectParams.AbilityLevel,
		EffectContextHandle);

	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DamageEffectParams.DamageType, DamageEffectParams.BaseDamage);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Chance, DamageEffectParams.DebuffChance);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Damage, DamageEffectParams.DebuffDamage);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Duration, DamageEffectParams.DebuffDuration);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Frequency, DamageEffectParams.DebuffFrequency);

	DamageEffectParams.TargetAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
	return EffectContextHandle;
}

TArray<FRotator> UDreamAbilitySystemLibrary::EvenlySpacedRotators(const FVector& Forward, const FVector& Axis, float Spread, int32 NumRotators)
{
	TArray<FRotator> Rotators;

	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis);
	if (NumRotators > 1)
	{
		const float DeltaSpread = Spread / (NumRotators - 1);
		for (int32 i = 0; i < NumRotators; i++)
		{
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread*i , FVector::UpVector);
			Rotators.Add(Direction.Rotation());
		}
	}
	else
	{
		Rotators.Add(Forward.Rotation());
	}

	return Rotators;
}

TArray<FVector> UDreamAbilitySystemLibrary::EvenlyRotatedVectors(const FVector& Forward, const FVector& Axis, float Spread, int32 NumVectors)
{
	TArray<FVector> Vectors;

	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis);
	if (NumVectors > 1)
	{
		const float  DeltaSpread = Spread / (NumVectors - 1);
		for (int32 i = 0; i < NumVectors; i++)
		{
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);
			Vectors.Add(Direction);
		}
	}
	else
	{
		Vectors.Add(Forward);
	}
	return Vectors;
}

int32 UDreamAbilitySystemLibrary::GetXPRewardForClassAndLevel(const UObject* WorldContextObject, ECharacterClass CharacterClass, int32 CharacterLevel)
{
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr) return 0;

	const FCharacterClassDefaultInfo& Info = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
	const float XPReward = Info.XPReward.GetValueAtLevel(CharacterLevel);	// ScalableFloat �̱� ������, Level�� �ش��ϴ� ���� ������.

	return static_cast<int32>(XPReward);
	return static_cast<int32>(XPReward);
}

void UDreamAbilitySystemLibrary::AddItem(AActor* Target, UAbilitySystemComponent* TargetASC, FGameplayTag& ItemTag)
{
	if (!Target)	return;

	if (Target->Implements<UPlayerInterface>())
	{
		IPlayerInterface* TargetInterface = Cast<IPlayerInterface>(Target);
		if (TargetInterface)
		{
			// Overlap�� Target�� PlayerCharacter Ŭ������ ���, ������ �߰�(AddToItem)
			IPlayerInterface::Execute_AddToItem(Target, ItemTag);
		}
	}
}

void UDreamAbilitySystemLibrary::AddGold(AActor* TargetActor, UAbilitySystemComponent* TargetASC, int32 Gold)
{
	if (!TargetActor)	return;

	if (TargetActor->Implements<UPlayerInterface>())
	{
		IPlayerInterface* TargetInterface = Cast<IPlayerInterface>(TargetActor);
		if (TargetInterface)
		{
			IPlayerInterface::Execute_AddToGold(TargetActor, Gold);
		}
	}

}
