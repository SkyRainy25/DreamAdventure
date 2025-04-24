// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "UObject/NoExportTypes.h"
#include "DreamWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChangedSignature, int32, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityInfoSignature, const FDreamAbilityInfo&, Info);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FItemInfoDelegate, const FDreamItemInfo&, ItemInfo);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FItemNumberChangedSignature, const FDreamItemInfo&, ItemInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FItemEquippedSignature, const FDreamItemInfo&, ItemInfo);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStoreItemInfoSignature, const FDreamItemInfo&, ItemInfo);

class UAttributeSet;
class UAbilitySystemComponent;
class ADreamPlayerController;
class ADreamPlayerState;
class UDreamAbilitySystemComponent;
class UDreamAttributeSet;
class UAbilityInfo;
class UItemInfo;
struct DreamItemInfo;


USTRUCT(BlueprintType)
struct FWidgetControllerParams
{
	GENERATED_BODY()

	FWidgetControllerParams() {}
	FWidgetControllerParams(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
		: PlayerController(PC), PlayerState(PS), AbilitySystemComponent(ASC), AttributeSet(AS) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerState> PlayerState = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAttributeSet> AttributeSet = nullptr;
};

/**
 * 
 */
UCLASS()
class DREAMADVENTURE_API UDreamWidgetController : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetWidgetControllerParams(const FWidgetControllerParams& WCParams);

	UFUNCTION(BlueprintCallable)
	virtual void BroadcastInitialValues();

	virtual void BindCallbacksToDependencies();

	UPROPERTY(BlueprintAssignable, Category = "GAS|Messages")
	FAbilityInfoSignature AbilityInfoDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Item")
	FItemInfoDelegate OnItemChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Item")
	FItemInfoDelegate OnEquippedItemDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Item")
	FItemInfoDelegate OnRemoveItemDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Item")
	FItemNumberChangedSignature OnHpPotionChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Item")
	FItemNumberChangedSignature OnItemNumberChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Item")
	FItemNumberChangedSignature OnMpPotionChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Item")
	FItemEquippedSignature OnItemEquippedDelegate;

	void BroadcastAbilityInfo();
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Data")
	TObjectPtr<UAbilityInfo> AbilityInfo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Data")
	TObjectPtr<UItemInfo> ItemInfo;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<APlayerState> PlayerState;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UAttributeSet> AttributeSet;


	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<ADreamPlayerController> DreamPlayerController;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<ADreamPlayerState> DreamPlayerState;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UDreamAbilitySystemComponent> DreamAbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UDreamAttributeSet> DreamAttributeSet;

	ADreamPlayerController* GetDreamPC();
	ADreamPlayerState* GetDreamPS();
	UDreamAbilitySystemComponent* GetDreamASC();
	UDreamAttributeSet* GetDreamAS();
};
