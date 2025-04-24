// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AbilityTasks/TargetDataUnderMouse.h"

#include "AbilitySystemComponent.h"
#include "DreamAdventure/DreamAdventure.h"

UTargetDataUnderMouse* UTargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility)
{
	UTargetDataUnderMouse* MyObj = NewAbilityTask<UTargetDataUnderMouse>(OwningAbility);
	return MyObj;
}

void UTargetDataUnderMouse::Activate()
{
	// 클라이언트인지 확인.
	const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
	if (bIsLocallyControlled)
	{
		SendMouseCursorData();
	}
	else
	{
		//TODO: We are on the server, so listen for target data.
		const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
		const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
		// return TargetData Delegate for a given Ability/ PredictionKey
		AbilitySystemComponent.Get()->AbilityTargetDataSetDelegate(
			SpecHandle, ActivationPredictionKey).AddUObject(
				this, &UTargetDataUnderMouse::OnTargetDataReplicatedCallback);

		// TargetData가 서버로 이미 보내졌는지(true) / 델리게이트가 이미 broadcast 했는지를 판별하기 위한 함수
		const bool bCalledDelegate = AbilitySystemComponent.Get()
			->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);

		// TargetData를 받지 않았다면 기다려라
		if (!bCalledDelegate)
		{
			SetWaitingOnRemotePlayerData();
		}
	}
}

void UTargetDataUnderMouse::SendMouseCursorData()
{
	// 해당 범위에 있는 코드들을 서버에서 예측 가능하도록 하는 코드
	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());

	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	FHitResult CursorHit;
	PC->GetHitResultUnderCursor(ECC_Target, false, CursorHit);
	
	// TargetHit에 대한 TargetData를 new 를 통해 생성
	FGameplayAbilityTargetDataHandle DataHandle;
	FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit();
	Data->HitResult = CursorHit;
	DataHandle.Add(Data);


	// 서버에 TargetData를 Replicate(Client -> Server)
	AbilitySystemComponent->ServerSetReplicatedTargetData(
		GetAbilitySpecHandle(),
		GetActivationPredictionKey(),
		DataHandle,
		FGameplayTag(),
		AbilitySystemComponent->ScopedPredictionKey);

	// AbilityTask가 Active 일 때만, Delegate를 통해 Broadcast
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}

void UTargetDataUnderMouse::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag)
{
	// Server -> Client로 ASC 등의 데이터를 받기 때문에, 받은 데이터를 저장하지 말고 지워라
	// 어차피 서버에서 관련 정보를 저장하고 있기 때문에.
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}
