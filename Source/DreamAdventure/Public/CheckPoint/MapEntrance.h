// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CheckPoint/Checkpoint.h"
#include "MapEntrance.generated.h"

/**
 * 
 */
UCLASS()
class DREAMADVENTURE_API AMapEntrance : public ACheckpoint
{
	GENERATED_BODY()
public:
	AMapEntrance(const FObjectInitializer& ObjectInitializer);

	/* Highlight Interface */
	virtual void HighlightActor_Implementation() override;
	/* Highlight Interface */

	/* Save Interface */
	virtual void LoadActor_Implementation() override;
	/* end Save Interface */

	// µµÂøÇÒ Map
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UWorld> DestinationMap;

	// µµÂøÇÒ MapÀÇ PlayerStartTag
	UPROPERTY(EditAnywhere)
	FName DestinationPlayerStartTag;

protected:

	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult) override;
};
