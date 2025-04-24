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
	LoadSlots.Add(0, LoadSlot_0);	// Tracking 하기 위해 LoadSlot을 index에 맞게 추가.
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
	/* 슬롯에 필요한 정보를 설정.*/

	// 맵 이름 설정.
	LoadSlots[Slot]->SetMapName(DreamGameMode->DefaultMapName);
	// 플레이어의 이름을 설정 -> MVVM 매크로에 의해 설정되고 변화를 Broadcast.
	LoadSlots[Slot]->SetPlayerName(EnteredName);
	LoadSlots[Slot]->SetPlayerLevel(1);
	LoadSlots[Slot]->SlotStatus = Taken;	// Status : Taken(저장됨) 으로 변경
	// 새로운 게임 시작시 기본 PlayerStart로 지정.
	LoadSlots[Slot]->PlayerStartTag = DreamGameMode->DefaultPlayerStartTag;	
	// MapAssetName 을 저장
	LoadSlots[Slot]->MapAssetName = DreamGameMode->DefaultMap.ToSoftObjectPath().GetAssetName();
	// 게임 저장.
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
	SlotSelected.Broadcast();	// LoadSlot이 눌렸음을 LoadScreen에 Broadcast
	for (const TTuple<int32, UMVVM_LoadSlot*> LoadSlot : LoadSlots)
	{
		if (LoadSlot.Key == Slot)
		{
			// 각 LoadSlot에 Broadcast
			LoadSlot.Value->EnableSelectSlotButton.Broadcast(false);	// 버튼 비활성화
		}
		else
		{
			LoadSlot.Value->EnableSelectSlotButton.Broadcast(true);		// 버튼 활성화
		}
	}
	SelectedSlot = LoadSlots[Slot];
}

void UMVVM_LoadScreen::DeleteButtonPressed()
{
	if (IsValid(SelectedSlot))
	{
		// 슬롯에 있는 Save파일 삭제.
		ADreamGameModeBase::DeleteSlot(SelectedSlot->GetLoadSlotName(), SelectedSlot->SlotIndex);
		SelectedSlot->SlotStatus = Vacant;
		SelectedSlot->InitializeSlot();
		// Slot을 삭제하고 다시 새로운 슬롯을 만들었을 때 Slot 버튼 활성화.
		SelectedSlot->EnableSelectSlotButton.Broadcast(true);	
	}
}

void UMVVM_LoadScreen::PlayButtonPressed()
{
	ADreamGameModeBase* DreamGameMode = Cast<ADreamGameModeBase>(UGameplayStatics::GetGameMode(this));
	UDreamGameInstance* DreamGameInstance = Cast<UDreamGameInstance>(DreamGameMode->GetGameInstance());

	// 플레이할 때, GameInstance에 정보를 채워넣음.
	DreamGameInstance->PlayerStartTag = SelectedSlot->PlayerStartTag;	// 선택한 슬롯의 시작 위치를 가져옴.
	DreamGameInstance->LoadSlotName = SelectedSlot->GetLoadSlotName();	// 슬롯의 이름
	DreamGameInstance->LoadSlotIndex = SelectedSlot->SlotIndex;			// 슬롯의 인덱스

	if (IsValid(SelectedSlot))
	{
		// 선택한 Slot의 Level을 Open
		DreamGameMode->TravelToMap(SelectedSlot);
	}
}

void UMVVM_LoadScreen::LoadData()
{
	// GameMode를 가져오고
	ADreamGameModeBase* DreamGameMode = Cast<ADreamGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (!IsValid(DreamGameMode)) return;
	for (const TTuple<int32, UMVVM_LoadSlot*> LoadSlot : LoadSlots)	// 순회/탐색
	{
		ULoadScreenSaveGame* SaveObject = DreamGameMode->GetSaveSlotData(LoadSlot.Value->GetLoadSlotName(), LoadSlot.Key);

		const FString PlayerName = SaveObject->PlayerName;	// 플레이어 이름 가져오고
		TEnumAsByte<ESaveSlotStatus> SaveSlotStatus = SaveObject->SaveSlotStatus;	// Status

		LoadSlot.Value->SlotStatus = SaveSlotStatus;	// Status 설정
		LoadSlot.Value->SetPlayerName(PlayerName);	// 이름 설정
		LoadSlot.Value->InitializeSlot();	//

		LoadSlot.Value->SetMapName(SaveObject->MapName);
		LoadSlot.Value->PlayerStartTag = SaveObject->PlayerStartTag;	// 저장한 위치(마지막 위치)의 PlayerStartTag를 저장.
		LoadSlot.Value->SetPlayerLevel(SaveObject->PlayerLevel);
	}
}

void UMVVM_LoadScreen::SetNumLoadSlots(int32 InNumLoadSlots)
{
	UE_MVVM_SET_PROPERTY_VALUE(NumLoadSlots, InNumLoadSlots);
}
