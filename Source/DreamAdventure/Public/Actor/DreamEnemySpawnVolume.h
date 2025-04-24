// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/SaveInterface.h"
#include "DreamEnemySpawnVolume.generated.h"

class ADreamEnemySpawnPoint;
class UBoxComponent;

UCLASS()
class DREAMADVENTURE_API ADreamEnemySpawnVolume : public AActor, public ISaveInterface
{
	GENERATED_BODY()
	
public:
	ADreamEnemySpawnVolume();

	/* Save Interface */
	virtual void LoadActor_Implementation() override;
	/* end Save Interface */

	UPROPERTY(BlueprintReadOnly, SaveGame)
	bool bReached = false;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere)
	TArray<ADreamEnemySpawnPoint*> SpawnPoints;
private:

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> Box;
};
