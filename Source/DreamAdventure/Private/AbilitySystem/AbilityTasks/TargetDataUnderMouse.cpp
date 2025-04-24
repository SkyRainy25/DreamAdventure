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
	// Ŭ���̾�Ʈ���� Ȯ��.
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

		// TargetData�� ������ �̹� ����������(true) / ��������Ʈ�� �̹� broadcast �ߴ����� �Ǻ��ϱ� ���� �Լ�
		const bool bCalledDelegate = AbilitySystemComponent.Get()
			->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);

		// TargetData�� ���� �ʾҴٸ� ��ٷ���
		if (!bCalledDelegate)
		{
			SetWaitingOnRemotePlayerData();
		}
	}
}

void UTargetDataUnderMouse::SendMouseCursorData()
{
	// �ش� ������ �ִ� �ڵ���� �������� ���� �����ϵ��� �ϴ� �ڵ�
	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());

	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	FHitResult CursorHit;
	PC->GetHitResultUnderCursor(ECC_Target, false, CursorHit);
	
	// TargetHit�� ���� TargetData�� new �� ���� ����
	FGameplayAbilityTargetDataHandle DataHandle;
	FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit();
	Data->HitResult = CursorHit;
	DataHandle.Add(Data);


	// ������ TargetData�� Replicate(Client -> Server)
	AbilitySystemComponent->ServerSetReplicatedTargetData(
		GetAbilitySpecHandle(),
		GetActivationPredictionKey(),
		DataHandle,
		FGameplayTag(),
		AbilitySystemComponent->ScopedPredictionKey);

	// AbilityTask�� Active �� ����, Delegate�� ���� Broadcast
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}

void UTargetDataUnderMouse::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag)
{
	// Server -> Client�� ASC ���� �����͸� �ޱ� ������, ���� �����͸� �������� ���� ������
	// ������ �������� ���� ������ �����ϰ� �ֱ� ������.
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}
