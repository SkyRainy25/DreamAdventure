// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PlayerCharacter.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystem/DreamAbilitySystemComponent.h"
#include "AbilitySystem/DreamAbilitySystemLibrary.h"
#include "Player/DreamPlayerController.h"
#include "AbilitySystem/DreamAttributeSet.h"
#include "Player/DreamPlayerState.h"
#include "UI/HUD/DreamHUD.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "DreamGameplayTags.h"
#include "AbilitySystem/Debuff/DebuffNiagaraComponent.h"

#include "NiagaraComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "AbilitySystem/Data/ItemInfo.h"

#include "Game/DreamGameInstance.h"
#include "Game/DreamGameModeBase.h"
#include "Game/LoadScreenSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystem/Data/AbilityInfo.h"

APlayerCharacter::APlayerCharacter()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->bDoCollisionTest = false;

	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>("TopDownCameraComponent");
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false;

	LevelUpNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("LevelUpNiagaraComponent");
	LevelUpNiagaraComponent->SetupAttachment(GetRootComponent());
	LevelUpNiagaraComponent->bAutoActivate = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	CharacterClass = ECharacterClass::Ranger;
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Init ability actor info for the Server
	InitAbilityActorInfo();
	LoadProgress();

	if (ADreamGameModeBase* DreamGameMode = Cast<ADreamGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		DreamGameMode->LoadWorldState(GetWorld());
	}
}

void APlayerCharacter::LoadProgress()
{
	ADreamGameModeBase* DreamGameMode = Cast<ADreamGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (DreamGameMode)
	{
		ULoadScreenSaveGame* SaveData = DreamGameMode->RetrieveInGameSaveData();
		if (SaveData == nullptr) return;

		// 처음 로딩을 했다면?
		if (SaveData->bFirstTimeLoadIn)
		{
			InitializeDefaultAttributes();
			AddCharacterAbilities();
		}
		else
		{
			// TODO : Load in Abilities from disk
			if (UDreamAbilitySystemComponent* DreamASC = Cast<UDreamAbilitySystemComponent>(AbilitySystemComponent))
			{
				DreamASC->AddCharacterAbilitiesFromSaveData(SaveData);
			}

			if (ADreamPlayerState* DreamPlayerState = Cast<ADreamPlayerState>(GetPlayerState()))
			{
				DreamPlayerState->SetLevel(SaveData->PlayerLevel);
				DreamPlayerState->SetXP(SaveData->XP);
				DreamPlayerState->SetAttributePoints(SaveData->AttributePoints);
				DreamPlayerState->SetSkillPoints(SaveData->SkillPoints);
				DreamPlayerState->SetGold(SaveData->Gold);
				// 가지고 있던 인벤토리를 가져옴.

				/* 문제가 될 수 있는 코드 */ 
				DreamPlayerState->SetInventory(SaveData->Inventory);
				for (const FDreamItemInfo& Item : SaveData->Inventory)
				{
					// SaveData에서 인벤토리에 저장된 아이템을 가져옴.
					DreamPlayerState->OnItemInfoDelegate.Broadcast(Item);
				}
			}

			UDreamAbilitySystemLibrary::InitializeDefaultAttributesFromSaveData(this, AbilitySystemComponent, SaveData);
		}
	}
}

void APlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Init ability actor info for the Client
	InitAbilityActorInfo();

}

void APlayerCharacter::AddToXP_Implementation(int32 InXP)
{
	ADreamPlayerState* DreamPlayerState = GetPlayerState<ADreamPlayerState>();
	check(DreamPlayerState);
	DreamPlayerState->AddToXP(InXP);
}

void APlayerCharacter::AddToItem_Implementation(FGameplayTag ItemTag)
{
	ADreamPlayerState* DreamPS = GetPlayerState<ADreamPlayerState>();
	check(DreamPS);
	DreamPS->AddToItem(ItemTag);
}

void APlayerCharacter::AddToGold_Implementation(int32 Gold)
{
	ADreamPlayerState* DreamPS = GetPlayerState<ADreamPlayerState>();
	check(DreamPS);
	DreamPS->AddToGold(Gold);
}

void APlayerCharacter::RemoveItem_Implementation(FGameplayTag ItemTag)
{
	ADreamPlayerState* DreamPS = GetPlayerState<ADreamPlayerState>();
	check(DreamPS);
	DreamPS->RemoveItem(ItemTag);
}

void APlayerCharacter::LevelUp_Implementation()
{
	MulticastLevelUpParticles();
}

int32 APlayerCharacter::GetXP_Implementation() const
{
	ADreamPlayerState* DreamPS = GetPlayerState<ADreamPlayerState>();
	check(DreamPS);
	return DreamPS->GetXP();
}

FDreamItemInfo APlayerCharacter::GetItem_Implementation() const
{

	return FDreamItemInfo();
}

int32 APlayerCharacter::FindLevelForXP_Implementation(int32 InXP) const
{
	ADreamPlayerState* DreamPS = GetPlayerState<ADreamPlayerState>();
	check(DreamPS);
	return DreamPS->LevelUpInfo->FindLevelForXP(InXP);
}

int32 APlayerCharacter::GetAttributePointsReward_Implementation(int32 Level) const
{
	ADreamPlayerState* DreamPS = GetPlayerState<ADreamPlayerState>();
	check(DreamPS);
	return DreamPS->LevelUpInfo->LevelUpInformation[Level].AttributePointAward;
}

int32 APlayerCharacter::GetSkillPointsReward_Implementation(int32 Level) const
{
	ADreamPlayerState* DreamPS = GetPlayerState<ADreamPlayerState>();
	check(DreamPS);
	return DreamPS->LevelUpInfo->LevelUpInformation[Level].SkillPointAward;
}

void APlayerCharacter::AddToPlayerLevel_Implementation(int32 InPlayerLevel)
{
	ADreamPlayerState* DreamPS = GetPlayerState<ADreamPlayerState>();
	check(DreamPS);
	DreamPS->AddToLevel(InPlayerLevel);

	if (UDreamAbilitySystemComponent* DreamASC = Cast<UDreamAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		// 플레이어 레벨업 시, 레벨에 따른 Ability 를 Update
		DreamASC->UpdateAbilityStatuses(DreamPS->GetPlayerLevel());
	}
}

void APlayerCharacter::AddToAttributePoints_Implementation(int32 InAttributePoints)
{
	ADreamPlayerState* DreamPS = GetPlayerState<ADreamPlayerState>();
	check(DreamPS);
	DreamPS->AddToAttributePoints(InAttributePoints);
}

void APlayerCharacter::AddToSkillPoints_Implementation(int32 InSkillPoints)
{
	ADreamPlayerState* DreamPS = GetPlayerState<ADreamPlayerState>();
	check(DreamPS);
	DreamPS->AddToSkillPoints(InSkillPoints);
}

int32 APlayerCharacter::GetAttributePoints_Implementation() const
{
	ADreamPlayerState* DreamPS = GetPlayerState<ADreamPlayerState>();
	check(DreamPS);
	return DreamPS->GetAttributePoints();
}

int32 APlayerCharacter::GetSkillPoints_Implementation() const
{
	ADreamPlayerState* DreamPS = GetPlayerState<ADreamPlayerState>();
	check(DreamPS);
	return DreamPS->GetSkillPoints();
}

void APlayerCharacter::ShowSKillCircle_Implementation(UMaterialInterface* DecalMaterial)
{
	if (ADreamPlayerController* DreamPC = Cast<ADreamPlayerController>(GetController()))
	{
		DreamPC->ShowSkillCircle(DecalMaterial);
		DreamPC->bShowMouseCursor = false;
	}
}

void APlayerCharacter::HideSkillCircle_Implementation()
{
	if (ADreamPlayerController* DreamPC = Cast<ADreamPlayerController>(GetController()))
	{
		DreamPC->HideSkillCircle();
		DreamPC->bShowMouseCursor = true;
	}
}

void APlayerCharacter::SaveProgress_Implementation(const FName& CheckpointTag)
{
	ADreamGameModeBase* DreamGameMode = Cast<ADreamGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (DreamGameMode)
	{
		ULoadScreenSaveGame* SaveData = DreamGameMode->RetrieveInGameSaveData();
		if (SaveData == nullptr) return;

		// 체크포인트에 있는 PlayerStart의 Tag를 가져와서 설정.
		SaveData->PlayerStartTag = CheckpointTag;

		if (ADreamPlayerState* DreamPlayerState = Cast<ADreamPlayerState>(GetPlayerState()))
		{
			SaveData->PlayerLevel = DreamPlayerState->GetPlayerLevel();
			SaveData->XP = DreamPlayerState->GetXP();
			SaveData->Gold = DreamPlayerState->GetGold();
			SaveData->AttributePoints = DreamPlayerState->GetAttributePoints();
			SaveData->SkillPoints = DreamPlayerState->GetSkillPoints();
			// 가지고 있던 인벤토리를 가져옴.
			SaveData->Inventory = DreamPlayerState->GetInventory();
		}
		// PrimaryAttribute를 가져옴.(GE)
		SaveData->Strength = UDreamAttributeSet::GetStrengthAttribute().GetNumericValue(GetAttributeSet());
		SaveData->Intelligence = UDreamAttributeSet::GetIntelligenceAttribute().GetNumericValue(GetAttributeSet());
		SaveData->Resilience = UDreamAttributeSet::GetResilienceAttribute().GetNumericValue(GetAttributeSet());
		SaveData->Vigor = UDreamAttributeSet::GetVigorAttribute().GetNumericValue(GetAttributeSet());

		SaveData->bFirstTimeLoadIn = false;

		// GameModeBase가 있는지 체크하기 위한 코드? -> GetAbilityInfo를 실행하기 위함.
		if (!HasAuthority()) return;

		UDreamAbilitySystemComponent* DreamASC = Cast<UDreamAbilitySystemComponent>(AbilitySystemComponent);
		// ForEachAbility를 사용하기 위한 변수
		FForEachAbility SaveAbilityDelegate;
		SaveData->SavedAbilities.Empty();
		SaveAbilityDelegate.BindLambda([this, DreamASC, SaveData](const FGameplayAbilitySpec& AbilitySpec)
			{
				// Spec을 통해 AbilityTag를 가져옴
				const FGameplayTag AbilityTag = DreamASC->GetAbilityTagFromSpec(AbilitySpec);
				// GameMode가 있어야 GA를 가져올 수 있음
				UAbilityInfo* AbilityInfo = UDreamAbilitySystemLibrary::GetAbilityInfo(this);
				// AbilityTag를 통해 GA를 가져옴.
				FDreamAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);

				// GameplayAbility에 필요한 데이터를 채워넣고
				FSavedAbility SavedAbility;
				SavedAbility.GameplayAbility = Info.Ability;
				SavedAbility.AbilityLevel = AbilitySpec.Level;
				SavedAbility.AbilitySlot = DreamASC->GetSlotFromAbilityTag(AbilityTag);
				SavedAbility.AbilityStatus = DreamASC->GetStatusFromAbilityTag(AbilityTag);
				SavedAbility.AbilityTag = AbilityTag;
				SavedAbility.AbilityType = Info.AbilityType;

				// 콜백함수 호출
				SaveData->SavedAbilities.AddUnique(SavedAbility);

			});
		DreamASC->ForEachAbility(SaveAbilityDelegate);

		DreamGameMode->SaveInGameProgressData(SaveData);
	}
}

void APlayerCharacter::LoadProgress_Implementation()
{
	LoadProgress();
}

int32 APlayerCharacter::GetGold_Implementation() const
{
	ADreamPlayerState* DreamPS = GetPlayerState<ADreamPlayerState>();
	check(DreamPS);
	return DreamPS->GetGold();
}

void APlayerCharacter::SetGold_Implementation(int32 InGold)
{
	ADreamPlayerState* DreamPS = GetPlayerState<ADreamPlayerState>();
	check(DreamPS);
	DreamPS->SetGold(InGold);
}

void APlayerCharacter::AddItemToStoreMenu_Implementation(const FGameplayTag& ItemTag, int32 ItemAmount)
{
	ADreamPlayerState* DreamPS = GetPlayerState<ADreamPlayerState>();
	check(DreamPS);
	DreamPS->AddItemToStoreMenu(ItemTag, ItemAmount);
}

int32 APlayerCharacter::GetPlayerLevel_Implementation()
{
	ADreamPlayerState* DreamPlayerState = GetPlayerState<ADreamPlayerState>();
	check(DreamPlayerState);
	return DreamPlayerState->GetPlayerLevel();
}

void APlayerCharacter::Die(const FVector& DeathImpulse)
{
	Super::Die(DeathImpulse);

	FTimerDelegate DeathTimerDelegate;
	DeathTimerDelegate.BindLambda([this]()
		{
			ADreamGameModeBase* DreamGM = Cast<ADreamGameModeBase>(UGameplayStatics::GetGameMode(this));
			if (DreamGM)
			{
				// 플레이어 사망 처리
				DreamGM->PlayerDied(this);
			}
		});
	// DeathTime만큼의 Timer 설정(false = Loop 방지)
	GetWorldTimerManager().SetTimer(DeathTimer, DeathTimerDelegate, DeathTime, false);
	// 사망시 카메라가 떨어지는 것을 방지. 카메라 위치 유지.
	TopDownCameraComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
}

void APlayerCharacter::OnRep_Stunned()
{
	if (UDreamAbilitySystemComponent* DreamASC = Cast<UDreamAbilitySystemComponent>(AbilitySystemComponent))
	{
		const FDreamGameplayTags& GameplayTags = FDreamGameplayTags::Get();
		FGameplayTagContainer BlockedTags;
		BlockedTags.AddTag(GameplayTags.Player_Block_CursorTrace);
		BlockedTags.AddTag(GameplayTags.Player_Block_InputHeld);
		BlockedTags.AddTag(GameplayTags.Player_Block_InputPressed);
		BlockedTags.AddTag(GameplayTags.Player_Block_InputReleased);
		if (bIsStunned)
		{
			DreamASC->AddLooseGameplayTags(BlockedTags);
			StunDebuffComponent->Activate();
		}
		else
		{
			DreamASC->RemoveLooseGameplayTags(BlockedTags);
			StunDebuffComponent->Deactivate();
		}
	}
}

void APlayerCharacter::OnRep_Burned()
{
	if (bIsBurned)
	{
		BurnDebuffComponent->Activate();
	}
	else
	{
		BurnDebuffComponent->Deactivate();
	}
}

void APlayerCharacter::InitAbilityActorInfo()
{
	ADreamPlayerState* DreamPlayerState = GetPlayerState<ADreamPlayerState>();
	check(DreamPlayerState);
	// DreamPlayerState : Owned Actor / this(PlayerCharacter) : AvatarActor => 관계 설정(구축)
	DreamPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(DreamPlayerState, this);
	Cast<UDreamAbilitySystemComponent>(DreamPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();
	AbilitySystemComponent = DreamPlayerState->GetAbilitySystemComponent();
	AttributeSet = DreamPlayerState->GetAttributeSet();
	OnAscRegistered.Broadcast(AbilitySystemComponent);	// ASC가 설정되었음을 Broadcast
	AbilitySystemComponent->RegisterGameplayTagEvent(FDreamGameplayTags::Get().Debuff_Stun, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &APlayerCharacter::StunTagChanged);

	// PC, PS, ASC, AS  모두 설정.
	if (ADreamPlayerController* DreamPC = Cast<ADreamPlayerController>(GetController()))
	{
		if (ADreamHUD* DreamHUD = Cast<ADreamHUD>(DreamPC->GetHUD()))
		{
			DreamHUD->InitOverlay(DreamPC, DreamPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}
	//InitializeDefaultAttributes();
}

void APlayerCharacter::MulticastLevelUpParticles_Implementation() const
{
	if (IsValid(LevelUpNiagaraComponent))
	{
		const FVector CameraLocation = TopDownCameraComponent->GetComponentLocation();
		const FVector NiagaraSystemLocation = LevelUpNiagaraComponent->GetComponentLocation();
		const FRotator ToCameraRotation = (CameraLocation - NiagaraSystemLocation).Rotation();
		LevelUpNiagaraComponent->SetWorldRotation(ToCameraRotation);
		LevelUpNiagaraComponent->Activate(true);
	}
}
