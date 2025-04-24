// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ViewModel/MVVM_LoadScreen.h"

#include "Game/DreamGameInstance.h"
#include "Game/DreamGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"

void UMVVM_LoadScreen::InitializeLoadSlots()
{
	LoadSlot_0 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	LoadSlot_0->SetLoadSlotName("LoadSlot_0");
	LoadSlot_0->SlotIndex = 0;
	LoadSlots.Add(0, LoadSlot_0);	// Tracking �ϱ� ���� LoadSlot�� index�� �°� �߰�.
	LoadSlot_1 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	LoadSlot_1->SetLoadSlotName("LoadSlot_1");
	LoadSlot_1->SlotIndex = 1;
	LoadSlots.Add(1, LoadSlot_1);
	LoadSlot_2 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	LoadSlot_2->SetLoadSlotName("LoadSlot_2");
	LoadSlot_2->SlotIndex = 2;
	LoadSlots.Add(2, LoadSlot_2);

	SetNumLoadSlots(LoadSlots.Num());
}

UMVVM_LoadSlot* UMVVM_LoadScreen::GetLoadSlotViewModelByIndex(int32 Index) const
{
	return LoadSlots.FindChecked(Index);
}

void UMVVM_LoadScreen::NewSlotButtonPressed(int32 Slot, const FString& EnteredName)
{
	ADreamGameModeBase* DreamGameMode = Cast<ADreamGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (!IsValid(DreamGameMode))
	{
		GEngine->AddOnScreenDebugMessage(1, 15.f, FColor::Magenta, FString("Please switch to Single Player"));
		return;
	}
	/* ���Կ� �ʿ��� ������ ����.*/

	// �� �̸� ����.
	LoadSlots[Slot]->SetMapName(DreamGameMode->DefaultMapName);
	// �÷��̾��� �̸��� ���� -> MVVM ��ũ�ο� ���� �����ǰ� ��ȭ�� Broadcast.
	LoadSlots[Slot]->SetPlayerName(EnteredName);
	LoadSlots[Slot]->SetPlayerLevel(1);
	LoadSlots[Slot]->SlotStatus = Taken;	// Status : Taken(�����) ���� ����
	// ���ο� ���� ���۽� �⺻ PlayerStart�� ����.
	LoadSlots[Slot]->PlayerStartTag = DreamGameMode->DefaultPlayerStartTag;	
	// MapAssetName �� ����
	LoadSlots[Slot]->MapAssetName = DreamGameMode->DefaultMap.ToSoftObjectPath().GetAssetName();
	// ���� ����.
	DreamGameMode->SaveSlotData(LoadSlots[Slot], Slot);
	LoadSlots[Slot]->InitializeSlot();

	UDreamGameInstance* DreamGameInstance = Cast<UDreamGameInstance>(DreamGameMode->GetGameInstance());
	DreamGameInstance->LoadSlotName = LoadSlots[Slot]->GetLoadSlotName();
	DreamGameInstance->LoadSlotIndex = LoadSlots[Slot]->SlotIndex;
	DreamGameInstance->PlayerStartTag = DreamGameMode->DefaultPlayerStartTag;
}

void UMVVM_LoadScreen::NewGameButtonPressed(int32 Slot)
{
	LoadSlots[Slot]->SetWidgetSwitcherIndex.Broadcast(1);
}

void UMVVM_LoadScreen::SelectSlotButtonPressed(int32 Slot)
{
	SlotSelected.Broadcast();	// LoadSlot�� �������� LoadScreen�� Broadcast
	for (const TTuple<int32, UMVVM_LoadSlot*> LoadSlot : LoadSlots)
	{
		if (LoadSlot.Key == Slot)
		{
			// �� LoadSlot�� Broadcast
			LoadSlot.Value->EnableSelectSlotButton.Broadcast(false);	// ��ư ��Ȱ��ȭ
		}
		else
		{
			LoadSlot.Value->EnableSelectSlotButton.Broadcast(true);		// ��ư Ȱ��ȭ
		}
	}
	SelectedSlot = LoadSlots[Slot];
}

void UMVVM_LoadScreen::DeleteButtonPressed()
{
	if (IsValid(SelectedSlot))
	{
		// ���Կ� �ִ� Save���� ����.
		ADreamGameModeBase::DeleteSlot(SelectedSlot->GetLoadSlotName(), SelectedSlot->SlotIndex);
		SelectedSlot->SlotStatus = Vacant;
		SelectedSlot->InitializeSlot();
		// Slot�� �����ϰ� �ٽ� ���ο� ������ ������� �� Slot ��ư Ȱ��ȭ.
		SelectedSlot->EnableSelectSlotButton.Broadcast(true);	
	}
}

void UMVVM_LoadScreen::PlayButtonPressed()
{
	ADreamGameModeBase* DreamGameMode = Cast<ADreamGameModeBase>(UGameplayStatics::GetGameMode(this));
	UDreamGameInstance* DreamGameInstance = Cast<UDreamGameInstance>(DreamGameMode->GetGameInstance());

	// �÷����� ��, GameInstance�� ������ ä������.
	DreamGameInstance->PlayerStartTag = SelectedSlot->PlayerStartTag;	// ������ ������ ���� ��ġ�� ������.
	DreamGameInstance->LoadSlotName = SelectedSlot->GetLoadSlotName();	// ������ �̸�
	DreamGameInstance->LoadSlotIndex = SelectedSlot->SlotIndex;			// ������ �ε���

	if (IsValid(SelectedSlot))
	{
		// ������ Slot�� Level�� Open
		DreamGameMode->TravelToMap(SelectedSlot);
	}
}

void UMVVM_LoadScreen::LoadData()
{
	// GameMode�� ��������
	ADreamGameModeBase* DreamGameMode = Cast<ADreamGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (!IsValid(DreamGameMode)) return;
	for (const TTuple<int32, UMVVM_LoadSlot*> LoadSlot : LoadSlots)	// ��ȸ/Ž��
	{
		ULoadScreenSaveGame* SaveObject = DreamGameMode->GetSaveSlotData(LoadSlot.Value->GetLoadSlotName(), LoadSlot.Key);

		const FString PlayerName = SaveObject->PlayerName;	// �÷��̾� �̸� ��������
		TEnumAsByte<ESaveSlotStatus> SaveSlotStatus = SaveObject->SaveSlotStatus;	// Status

		LoadSlot.Value->SlotStatus = SaveSlotStatus;	// Status ����
		LoadSlot.Value->SetPlayerName(PlayerName);	// �̸� ����
		LoadSlot.Value->InitializeSlot();	//

		LoadSlot.Value->SetMapName(SaveObject->MapName);
		LoadSlot.Value->PlayerStartTag = SaveObject->PlayerStartTag;	// ������ ��ġ(������ ��ġ)�� PlayerStartTag�� ����.
		LoadSlot.Value->SetPlayerLevel(SaveObject->PlayerLevel);
	}
}

void UMVVM_LoadScreen::SetNumLoadSlots(int32 InNumLoadSlots)
{
	UE_MVVM_SET_PROPERTY_VALUE(NumLoadSlots, InNumLoadSlots);
}
