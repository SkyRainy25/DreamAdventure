// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "DreamItem.generated.h"

class UGameplayEffect;
class UAbilitySystemComponent;

UCLASS()
class DREAMADVENTURE_API ADreamItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADreamItem();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void OnOverlap(AActor* TargetActor);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Information")
	FGameplayTag ItemTag;
};
