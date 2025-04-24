 // Fill out your copyright notice in the Description page of Project Settings.


#include "Character/EnemyCharacter.h"

#include "AbilitySystem/DreamAbilitySystemComponent.h"
#include "AbilitySystem/DreamAbilitySystemLibrary.h"
#include "AbilitySystem/DreamAttributeSet.h"
#include "Components/WidgetComponent.h"
#include "DreamAdventure/DreamAdventure.h"
#include "UI/Widget/DreamUserWidget.h"
#include "DreamGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "AI/DreamAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

AEnemyCharacter::AEnemyCharacter()
{
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	AbilitySystemComponent = CreateDefaultSubobject<UDreamAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	// GE is not Replicated, but GameplayCue & GameplayTags are Replicated to Clients
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UDreamAttributeSet>("AttributeSet");

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;

	HealthBar = CreateDefaultSubobject<UWidgetComponent>("HealthBar");
	HealthBar->SetupAttachment(GetRootComponent());

	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	GetMesh()->MarkRenderStateDirty();
	SkeletalWeapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	SkeletalWeapon->MarkRenderStateDirty();
	StaticWeapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	StaticWeapon->MarkRenderStateDirty();

	BaseWalkSpeed = 250.f;
}

void AEnemyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!HasAuthority())	return;
	DreamAIController = Cast<ADreamAIController>(NewController);
	DreamAIController->GetBlackboardComponent()->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
	DreamAIController->RunBehaviorTree(BehaviorTree);
	// HitReacting에 해당하는 Blackboard 키의 값을 설정
	DreamAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), false);
	DreamAIController->GetBlackboardComponent()->SetValueAsBool(FName("RangedAttacker"), CharacterClass != ECharacterClass::Warrior);
}

void AEnemyCharacter::HighlightActor_Implementation()
{
	GetMesh()->SetRenderCustomDepth(true);
	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	if (SkeletalWeapon)
	{
		SkeletalWeapon->SetRenderCustomDepth(true);
		SkeletalWeapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	}
	if (StaticWeapon)
	{
		StaticWeapon->SetRenderCustomDepth(true);
		StaticWeapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	}

}

void AEnemyCharacter::UnHighlightActor_Implementation()
{
	GetMesh()->SetRenderCustomDepth(false);
	if (SkeletalWeapon)
	{
		SkeletalWeapon->SetRenderCustomDepth(false);
	}
	if (StaticWeapon)
	{
		StaticWeapon->SetRenderCustomDepth(false);
	}

}

void AEnemyCharacter::SetMoveToLocation_Implementation(FVector& OutDestination)
{
}

int32 AEnemyCharacter::GetPlayerLevel_Implementation()
{
	return Level;
}

void AEnemyCharacter::Die(const FVector& DeathImpulse)
{
	SetLifeSpan(LifeSpan);

	if (DreamAIController) DreamAIController->GetBlackboardComponent()->SetValueAsBool(FName("Dead"), true);
	if(CharacterClass != ECharacterClass::Dragon)
		SpawnLoot();	// 사망시 아이템 드랍

	Super::Die(DeathImpulse);
}

void AEnemyCharacter::SetCombatTarget_Implementation(AActor* InCombatTarget)
{
	CombatTarget = InCombatTarget;
}

AActor* AEnemyCharacter::GetCombatTarget_Implementation() const
{
	return CombatTarget;
}

void AEnemyCharacter::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bHitReacting = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bHitReacting ? 0.f : BaseWalkSpeed;
	if (DreamAIController && DreamAIController->GetBlackboardComponent())
	{
		DreamAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), bHitReacting);
	}
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	InitAbilityActorInfo();
	if (HasAuthority())
	{
		UDreamAbilitySystemLibrary::GiveStartupAbilities(this, AbilitySystemComponent, CharacterClass);
	}


	if (UDreamUserWidget* DreamWidget = Cast<UDreamUserWidget>(HealthBar->GetUserWidgetObject()))
	{
		DreamWidget->SetWidgetController(this);
	}
	if (const UDreamAttributeSet* DreamAS = Cast<UDreamAttributeSet>(AttributeSet))
	{
		// Attribute의 값이 변하면 델리게이트에 등록?
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(DreamAS->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			}
		);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(DreamAS->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			}
		);

		AbilitySystemComponent->RegisterGameplayTagEvent(FDreamGameplayTags::Get().Effects_HitReact, EGameplayTagEventType::NewOrRemoved).AddUObject(
			this, &AEnemyCharacter::HitReactTagChanged);
		
		// 초기값 Broadcast
		OnHealthChanged.Broadcast(DreamAS->GetHealth());
		OnMaxHealthChanged.Broadcast(DreamAS->GetMaxHealth());
	}

}

void AEnemyCharacter::InitAbilityActorInfo()
{
	// AvatarActor : EnemyCharacter / Owned Actor : EnemyCharacter
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<UDreamAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
	AbilitySystemComponent->RegisterGameplayTagEvent(FDreamGameplayTags::Get().Debuff_Stun, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AEnemyCharacter::StunTagChanged);

	if (HasAuthority())
	{
		InitializeDefaultAttributes();
	}
	OnAscRegistered.Broadcast(AbilitySystemComponent);

}

void AEnemyCharacter::InitializeDefaultAttributes() const
{
	UDreamAbilitySystemLibrary::InitializeDefaultAttributes(this, CharacterClass, Level, AbilitySystemComponent);
}

void AEnemyCharacter::StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	Super::StunTagChanged(CallbackTag, NewCount);

	if (DreamAIController && DreamAIController->GetBlackboardComponent())
	{
		DreamAIController->GetBlackboardComponent()->SetValueAsBool(FName("Stunned"), bIsStunned);
	}
}
