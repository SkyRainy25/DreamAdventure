// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/DreamHUD.h"

#include "UI/Widget/DreamUserWidget.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "UI/WidgetController/AttributeMenuWidgetController.h"
#include "UI/WidgetController/SkillMenuWidgetController.h"
#include "UI/WidgetController/InventoryWidgetController.h"
#include "UI/WidgetController/StoreWidgetController.h"


UOverlayWidgetController* ADreamHUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	if (OverlayWidgetController == nullptr)
	{
		OverlayWidgetController = NewObject<UOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(WCParams);
		OverlayWidgetController->BindCallbacksToDependencies();	// OverlayWC를 생성하고 바인딩.
	}
	return OverlayWidgetController;
}

UAttributeMenuWidgetController* ADreamHUD::GetAttributeMenuWidgetController(const FWidgetControllerParams& WCParams)
{
	if (AttributeMenuWidgetController == nullptr)
	{
		AttributeMenuWidgetController = NewObject<UAttributeMenuWidgetController>(this, AttributeMenuWidgetControllerClass);
		AttributeMenuWidgetController->SetWidgetControllerParams(WCParams);
		AttributeMenuWidgetController->BindCallbacksToDependencies();
	}
	return AttributeMenuWidgetController;
}

USkillMenuWidgetController* ADreamHUD::GetSkillMenuWidgetController(const FWidgetControllerParams& WCParams)
{
	if (SkilllMenuWidgetController == nullptr)
	{
		SkilllMenuWidgetController = NewObject<USkillMenuWidgetController>(this, SkilllMenuWidgetControllerClass);
		SkilllMenuWidgetController->SetWidgetControllerParams(WCParams);
		SkilllMenuWidgetController->BindCallbacksToDependencies();
	}
	return SkilllMenuWidgetController;
}

UInventoryWidgetController* ADreamHUD::GetInventoryWidgetController(const FWidgetControllerParams& WCParams)
{
	if (InventoryWidgetController == nullptr)
	{
		InventoryWidgetController = NewObject<UInventoryWidgetController>(this, InventoryWidgetControllerClass);
		InventoryWidgetController->SetWidgetControllerParams(WCParams);
		InventoryWidgetController->BindCallbacksToDependencies();
	}
	return InventoryWidgetController;
}

UStoreWidgetController* ADreamHUD::GetStoreWidgetController(const FWidgetControllerParams& WCParams)
{
	if (StoreWidgetController == nullptr)
	{
		StoreWidgetController = NewObject<UStoreWidgetController>(this, StoreWidgetControllerClass);
		StoreWidgetController->SetWidgetControllerParams(WCParams);
		StoreWidgetController->BindCallbacksToDependencies();
	}
	return StoreWidgetController;
}

void ADreamHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(OverlayWidgetClass, TEXT("Overlay Widget Class uninitialized, please fill out BP_DreamHUD"));
	checkf(OverlayWidgetControllerClass, TEXT("Overlay Widget Controller Class uninitialized, please fill out BP_DreamHUD"));

	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
	OverlayWidget = Cast<UDreamUserWidget>(Widget);

	const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
	UOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);

	OverlayWidget->SetWidgetController(WidgetController);
	WidgetController->BroadcastInitialValues();

	Widget->AddToViewport();
}
