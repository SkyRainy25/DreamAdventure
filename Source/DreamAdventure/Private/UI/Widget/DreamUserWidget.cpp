// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/DreamUserWidget.h"

void UDreamUserWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	WidgetControllerSet();
}
