// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/DreamGameModeBase.h"

#include "EngineUtils.h"
#include "Game/DreamGameInstance.h"
#include "Game/LoadScreenSaveGame.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"
#include "Interaction/SaveInterface.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "GameFramework/Character.h"

void ADreamGameModeBase::SaveSlotData(UMVVM_LoadSlot* LoadSlot, int32 SlotIndex)
{
	// 이미 해당 인덱스에 Save파일이 있는 경우
	if (UGameplayStatics::DoesSaveGameExist(LoadSlot->GetLoadSlotName(), SlotIndex))
	{
		// 삭제 후 다시 생성.
		UGameplayStatics::DeleteGameInSlot(LoadSlot->GetLoadSlotName(), SlotIndex);
	}
	USaveGame* SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadScreenSaveGameClass);
	ULoadScreenSaveGame* LoadScreenSaveGame = Cast<ULoadScreenSaveGame>(SaveGameObject);
	LoadScreenSaveGame->PlayerName = LoadSlot->GetPlayerName();
	LoadScreenSaveGame->MapName = LoadSlot->GetMapName();
	LoadScreenSaveGame->MapAssetName = LoadSlot->MapAssetName;	// MapAssetName 저장-> 죽었을 경우 사용하기 위함.
	LoadScreenSaveGame->SaveSlotStatus = Taken;
	LoadScreenSaveGame->PlayerStartTag = LoadSlot->PlayerStartTag;

	// 실제 게임을 저장하는 코드.
	UGameplayStatics::SaveGameToSlot(LoadScreenSaveGame, LoadSlot->GetLoadSlotName(), SlotIndex);
}

ULoadScreenSaveGame* ADreamGameModeBase::GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const
{
	USaveGame* SaveGameObject = nullptr;
	if (UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex))	// 이미 저장된 게임이 있다면?
	{
		SaveGameObject = UGameplayStatics::LoadGameFromSlot(SlotName, SlotIndex);	// 불러오기
	}
	else
	{
		SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadScreenSaveGameClass);	// 새로 저장 파일 만들기
	}
	ULoadScreenSaveGame* LoadScreenSaveGame = Cast<ULoadScreenSaveGame>(SaveGameObject);
	return LoadScreenSaveGame;

}

void ADreamGameModeBase::DeleteSlot(const FString& SlotName, int32 SlotIndex)
{
	if (UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex))
	{
		UGameplayStatics::DeleteGameInSlot(SlotName, SlotIndex);	// Slot에 있는 파일 삭제.
	}
}

ULoadScreenSaveGame* ADreamGameModeBase::RetrieveInGameSaveData()
{
	UDreamGameInstance* DreamGameInstance = Cast<UDreamGameInstance>(GetGameInstance());

	// SaveSlotData에 필요한 데이터를 가져옴.
	const FString InGameLoadSlotName = DreamGameInstance->LoadSlotName;
	const int32 InGameLoadSlotIndex = DreamGameInstance->LoadSlotIndex;

	// 저장된 게임 슬롯 데이터를 가져옴.
	return GetSaveSlotData(InGameLoadSlotName, InGameLoadSlotIndex);
}

void ADreamGameModeBase::SaveInGameProgressData(ULoadScreenSaveGame* SaveObject)
{
	UDreamGameInstance* DreamGameInstance = Cast<UDreamGameInstance>(GetGameInstance());

	const FString InGameLoadSlotName = DreamGameInstance->LoadSlotName;
	const int32 InGameLoadSlotIndex = DreamGameInstance->LoadSlotIndex;
	DreamGameInstance->PlayerStartTag = SaveObject->PlayerStartTag;

	// 체크포인트 위치를 저장하고, 게임 슬롯에 Save파일 저장.
	UGameplayStatics::SaveGameToSlot(SaveObject, InGameLoadSlotName, InGameLoadSlotIndex);
}

void ADreamGameModeBase::SaveWorldState(UWorld* World, const FString& DestinationMapAssetName) const
{
	FString WorldName = World->GetMapName();	// 월드 이름을 가져오고
	WorldName.RemoveFromStart(World->StreamingLevelsPrefix);
	UDreamGameInstance* DreamGI = Cast<UDreamGameInstance>(GetGameInstance());
	check(DreamGI);
	// 선택한 슬롯의 Save 파일을 가져오고
	if (ULoadScreenSaveGame* SaveGame = GetSaveSlotData(DreamGI->LoadSlotName, DreamGI->LoadSlotIndex))
	{
		if (DestinationMapAssetName != FString(""))
		{
			// 도착할 맵의 이름과 에셋 이름을 저장.
			SaveGame->MapAssetName = DestinationMapAssetName;
			SaveGame->MapName = GetMapNameFromMapAssetName(DestinationMapAssetName);
		}

		// 세이브 파일에 맵이 저장되어 있지 않다면? -> 저장
		if (!SaveGame->HasMap(WorldName))	
		{
			FSavedMap NewSavedMap;
			NewSavedMap.MapAssetName = WorldName;
			SaveGame->SavedMaps.Add(NewSavedMap);
		}
		FSavedMap SavedMap = SaveGame->GetSavedMapWithMapName(WorldName);
		SavedMap.SavedActors.Empty(); // clear it out, we'll fill it in with "actors"

		// 월드에 있는 모든 액터를 순회 탐색.
		for (FActorIterator It(World); It; ++It)
		{
			AActor* Actor = *It;	// 액터의 포인터를 가져옴

			// 유효한 액터 X or SaveInterface 실행 X? -> 패스
			if (!IsValid(Actor) || !Actor->Implements<USaveInterface>()) continue;

			FSavedActor SavedActor;
			SavedActor.ActorName = Actor->GetFName();		// 액터 이름
			SavedActor.Transform = Actor->GetTransform();	// 액터 위치

			// 임의의 데이터를 정해진 메모리 위치에 저장하기 위한 Archive
			// Archive : Base class for Serializing arbirary data in memory
			FMemoryWriter MemoryWriter(SavedActor.Bytes);

			FObjectAndNameAsStringProxyArchive Archive(MemoryWriter, true);
			Archive.ArIsSaveGame = true;	// SaveGameObject를 통해 Serialize

			// 액터 Serialize
			Actor->Serialize(Archive);

			// 저장해야할 액터 목록에 추가.
			SavedMap.SavedActors.AddUnique(SavedActor);
		}

		for (FSavedMap& MapToReplace : SaveGame->SavedMaps)
		{
			if (MapToReplace.MapAssetName == WorldName)
			{
				MapToReplace = SavedMap;	// 맵 최신화.
			}
		}
		UGameplayStatics::SaveGameToSlot(SaveGame, DreamGI->LoadSlotName, DreamGI->LoadSlotIndex);
	}
}

void ADreamGameModeBase::LoadWorldState(UWorld* World) const
{
	FString WorldName = World->GetMapName();
	WorldName.RemoveFromStart(World->StreamingLevelsPrefix);

	UDreamGameInstance* DreamGI = Cast<UDreamGameInstance>(GetGameInstance());
	check(DreamGI);

	// 세이브 파일이 있는지 체크?
	if (UGameplayStatics::DoesSaveGameExist(DreamGI->LoadSlotName, DreamGI->LoadSlotIndex))
	{
		// 세이브 파일을 가져옴.
		ULoadScreenSaveGame* SaveGame = Cast<ULoadScreenSaveGame>(UGameplayStatics::LoadGameFromSlot(DreamGI->LoadSlotName, DreamGI->LoadSlotIndex));
		if (SaveGame == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to load slot"));
			return;
		}

		for (FActorIterator It(World); It; ++It)
		{
			AActor* Actor = *It;

			if (!Actor->Implements<USaveInterface>()) continue;

			// FSaveMap에서 저장된 액터를 순회 탐색
			for (FSavedActor SavedActor : SaveGame->GetSavedMapWithMapName(WorldName).SavedActors)
			{
				if (SavedActor.ActorName == Actor->GetFName())
				{
					if (ISaveInterface::Execute_ShouldLoadTransform(Actor))
					{
						// 위치 설정
						Actor->SetActorTransform(SavedActor.Transform);
					}

					FMemoryReader MemoryReader(SavedActor.Bytes);

					FObjectAndNameAsStringProxyArchive Archive(MemoryReader, true);
					Archive.ArIsSaveGame = true;
					Actor->Serialize(Archive); // converts binary bytes back into variables

					ISaveInterface::Execute_LoadActor(Actor);
				}
			}
		}
	}
}

void ADreamGameModeBase::TravelToMap(UMVVM_LoadSlot* Slot)
{
	// OpenLevelBySoftObjectPtr() 에 필요한 데이터 Get.
	const FString SlotName = Slot->GetLoadSlotName();
	const int32 SlotIndex = Slot->SlotIndex;

	// 슬롯의 이름과 Index를 통해 열어야할 Level(맵)을 찾아 Open
	UGameplayStatics::OpenLevelBySoftObjectPtr(Slot, Maps.FindChecked(Slot->GetMapName()));
}

FString ADreamGameModeBase::GetMapNameFromMapAssetName(const FString& MapAssetName) const
{
	for (auto& Map : Maps)
	{
		if (Map.Value.ToSoftObjectPath().GetAssetName() == MapAssetName)
		{
			return Map.Key;
		}
	}
	return FString();
}

AActor* ADreamGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	UDreamGameInstance* DreamGameInstance = Cast<UDreamGameInstance>(GetGameInstance());

	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), Actors);
	if (Actors.Num() > 0)
	{
		AActor* SelectedActor = Actors[0];
		for (AActor* Actor : Actors)
		{
			if (APlayerStart* PlayerStart = Cast<APlayerStart>(Actor))
			{
				if (PlayerStart->PlayerStartTag == DreamGameInstance->PlayerStartTag)
				{
					SelectedActor = PlayerStart;
					break;
				}
			}
		}
		return SelectedActor;
	}
	return nullptr;	
}

void ADreamGameModeBase::PlayerDied(ACharacter* DeadCharacter)
{
	ULoadScreenSaveGame* SaveGame = RetrieveInGameSaveData();
	if (!IsValid(SaveGame)) return;

	// 죽었을 때, Slot의 Save파일로부터 MapAssetName을 통해 Map을 불러옴.
	UGameplayStatics::OpenLevel(DeadCharacter, FName(SaveGame->MapAssetName));
}

void ADreamGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	Maps.Add(DefaultMapName, DefaultMap);
}
