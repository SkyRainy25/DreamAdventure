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
	// �̹� �ش� �ε����� Save������ �ִ� ���
	if (UGameplayStatics::DoesSaveGameExist(LoadSlot->GetLoadSlotName(), SlotIndex))
	{
		// ���� �� �ٽ� ����.
		UGameplayStatics::DeleteGameInSlot(LoadSlot->GetLoadSlotName(), SlotIndex);
	}
	USaveGame* SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadScreenSaveGameClass);
	ULoadScreenSaveGame* LoadScreenSaveGame = Cast<ULoadScreenSaveGame>(SaveGameObject);
	LoadScreenSaveGame->PlayerName = LoadSlot->GetPlayerName();
	LoadScreenSaveGame->MapName = LoadSlot->GetMapName();
	LoadScreenSaveGame->MapAssetName = LoadSlot->MapAssetName;	// MapAssetName ����-> �׾��� ��� ����ϱ� ����.
	LoadScreenSaveGame->SaveSlotStatus = Taken;
	LoadScreenSaveGame->PlayerStartTag = LoadSlot->PlayerStartTag;

	// ���� ������ �����ϴ� �ڵ�.
	UGameplayStatics::SaveGameToSlot(LoadScreenSaveGame, LoadSlot->GetLoadSlotName(), SlotIndex);
}

ULoadScreenSaveGame* ADreamGameModeBase::GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const
{
	USaveGame* SaveGameObject = nullptr;
	if (UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex))	// �̹� ����� ������ �ִٸ�?
	{
		SaveGameObject = UGameplayStatics::LoadGameFromSlot(SlotName, SlotIndex);	// �ҷ�����
	}
	else
	{
		SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadScreenSaveGameClass);	// ���� ���� ���� �����
	}
	ULoadScreenSaveGame* LoadScreenSaveGame = Cast<ULoadScreenSaveGame>(SaveGameObject);
	return LoadScreenSaveGame;

}

void ADreamGameModeBase::DeleteSlot(const FString& SlotName, int32 SlotIndex)
{
	if (UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex))
	{
		UGameplayStatics::DeleteGameInSlot(SlotName, SlotIndex);	// Slot�� �ִ� ���� ����.
	}
}

ULoadScreenSaveGame* ADreamGameModeBase::RetrieveInGameSaveData()
{
	UDreamGameInstance* DreamGameInstance = Cast<UDreamGameInstance>(GetGameInstance());

	// SaveSlotData�� �ʿ��� �����͸� ������.
	const FString InGameLoadSlotName = DreamGameInstance->LoadSlotName;
	const int32 InGameLoadSlotIndex = DreamGameInstance->LoadSlotIndex;

	// ����� ���� ���� �����͸� ������.
	return GetSaveSlotData(InGameLoadSlotName, InGameLoadSlotIndex);
}

void ADreamGameModeBase::SaveInGameProgressData(ULoadScreenSaveGame* SaveObject)
{
	UDreamGameInstance* DreamGameInstance = Cast<UDreamGameInstance>(GetGameInstance());

	const FString InGameLoadSlotName = DreamGameInstance->LoadSlotName;
	const int32 InGameLoadSlotIndex = DreamGameInstance->LoadSlotIndex;
	DreamGameInstance->PlayerStartTag = SaveObject->PlayerStartTag;

	// üũ����Ʈ ��ġ�� �����ϰ�, ���� ���Կ� Save���� ����.
	UGameplayStatics::SaveGameToSlot(SaveObject, InGameLoadSlotName, InGameLoadSlotIndex);
}

void ADreamGameModeBase::SaveWorldState(UWorld* World, const FString& DestinationMapAssetName) const
{
	FString WorldName = World->GetMapName();	// ���� �̸��� ��������
	WorldName.RemoveFromStart(World->StreamingLevelsPrefix);
	UDreamGameInstance* DreamGI = Cast<UDreamGameInstance>(GetGameInstance());
	check(DreamGI);
	// ������ ������ Save ������ ��������
	if (ULoadScreenSaveGame* SaveGame = GetSaveSlotData(DreamGI->LoadSlotName, DreamGI->LoadSlotIndex))
	{
		if (DestinationMapAssetName != FString(""))
		{
			// ������ ���� �̸��� ���� �̸��� ����.
			SaveGame->MapAssetName = DestinationMapAssetName;
			SaveGame->MapName = GetMapNameFromMapAssetName(DestinationMapAssetName);
		}

		// ���̺� ���Ͽ� ���� ����Ǿ� ���� �ʴٸ�? -> ����
		if (!SaveGame->HasMap(WorldName))	
		{
			FSavedMap NewSavedMap;
			NewSavedMap.MapAssetName = WorldName;
			SaveGame->SavedMaps.Add(NewSavedMap);
		}
		FSavedMap SavedMap = SaveGame->GetSavedMapWithMapName(WorldName);
		SavedMap.SavedActors.Empty(); // clear it out, we'll fill it in with "actors"

		// ���忡 �ִ� ��� ���͸� ��ȸ Ž��.
		for (FActorIterator It(World); It; ++It)
		{
			AActor* Actor = *It;	// ������ �����͸� ������

			// ��ȿ�� ���� X or SaveInterface ���� X? -> �н�
			if (!IsValid(Actor) || !Actor->Implements<USaveInterface>()) continue;

			FSavedActor SavedActor;
			SavedActor.ActorName = Actor->GetFName();		// ���� �̸�
			SavedActor.Transform = Actor->GetTransform();	// ���� ��ġ

			// ������ �����͸� ������ �޸� ��ġ�� �����ϱ� ���� Archive
			// Archive : Base class for Serializing arbirary data in memory
			FMemoryWriter MemoryWriter(SavedActor.Bytes);

			FObjectAndNameAsStringProxyArchive Archive(MemoryWriter, true);
			Archive.ArIsSaveGame = true;	// SaveGameObject�� ���� Serialize

			// ���� Serialize
			Actor->Serialize(Archive);

			// �����ؾ��� ���� ��Ͽ� �߰�.
			SavedMap.SavedActors.AddUnique(SavedActor);
		}

		for (FSavedMap& MapToReplace : SaveGame->SavedMaps)
		{
			if (MapToReplace.MapAssetName == WorldName)
			{
				MapToReplace = SavedMap;	// �� �ֽ�ȭ.
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

	// ���̺� ������ �ִ��� üũ?
	if (UGameplayStatics::DoesSaveGameExist(DreamGI->LoadSlotName, DreamGI->LoadSlotIndex))
	{
		// ���̺� ������ ������.
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

			// FSaveMap���� ����� ���͸� ��ȸ Ž��
			for (FSavedActor SavedActor : SaveGame->GetSavedMapWithMapName(WorldName).SavedActors)
			{
				if (SavedActor.ActorName == Actor->GetFName())
				{
					if (ISaveInterface::Execute_ShouldLoadTransform(Actor))
					{
						// ��ġ ����
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
	// OpenLevelBySoftObjectPtr() �� �ʿ��� ������ Get.
	const FString SlotName = Slot->GetLoadSlotName();
	const int32 SlotIndex = Slot->SlotIndex;

	// ������ �̸��� Index�� ���� ������� Level(��)�� ã�� Open
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

	// �׾��� ��, Slot�� Save���Ϸκ��� MapAssetName�� ���� Map�� �ҷ���.
	UGameplayStatics::OpenLevel(DeadCharacter, FName(SaveGame->MapAssetName));
}

void ADreamGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	Maps.Add(DefaultMapName, DefaultMap);
}
