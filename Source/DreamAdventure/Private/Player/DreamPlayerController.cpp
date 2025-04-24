// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/DreamPlayerController.h"

#include "DreamAdventure/DreamAdventure.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "DreamGameplayTags.h"
#include "AbilitySystem/DreamAbilitySystemComponent.h"
#include "Components/SplineComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Input/DreamInputComponent.h"
#include "Interaction/EnemyInterface.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "NiagaraFunctionLibrary.h"

#include "GameFramework/Character.h"
#include "UI/Widget/FloatTextComponent.h"
#include "Interaction/HighlightInterface.h"
#include "Components/DecalComponent.h"
#include "Actor/SkillCircle.h"

ADreamPlayerController::ADreamPlayerController()
{
	bReplicates = true;

	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

void ADreamPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CursorTrace();
	AutoRun();
	UpdateMagicCircleLocation();
}

void ADreamPlayerController::ShowSkillCircle(UMaterialInterface* DecalMaterial)
{
	if (!IsValid(SkillCircle))
	{
		SkillCircle = GetWorld()->SpawnActor<ASkillCircle>(SkillCircleClass);
		if (DecalMaterial)
		{
			SkillCircle->SkillCircleDecal->SetMaterial(0, DecalMaterial);
		}
	}
}

void ADreamPlayerController::HideSkillCircle()
{
	if (IsValid(SkillCircle))
	{
		SkillCircle->Destroy();
	}
}

void ADreamPlayerController::ShowDamageNumber_Implementation(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit)
{
	if (IsValid(TargetCharacter) && DamageTextComponentClass && IsLocalController())
	{
		UFloatTextComponent* DamageText = NewObject<UFloatTextComponent>(TargetCharacter, DamageTextComponentClass);
		DamageText->RegisterComponent();
		DamageText->AttachToComponent(TargetCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		DamageText->SetDamageText(DamageAmount, bBlockedHit, bCriticalHit);
	}
}

void ADreamPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(DreamInputContext);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(DreamInputContext, 0);
	}

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void ADreamPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UDreamInputComponent* DreamInputComponent = CastChecked<UDreamInputComponent>(InputComponent);
	DreamInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADreamPlayerController::Move);
	// DreamInputComponent�� ���ø��Լ��� ���ε�.
	DreamInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
}


void ADreamPlayerController::Move(const FInputActionValue& InputActionValue)
{
	// Press�� �� �� ���ٸ�.(Charging��ų�� �����)
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FDreamGameplayTags::Get().Player_Block_InputPressed))
	{
		return;
	}
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}

void ADreamPlayerController::CursorTrace()
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FDreamGameplayTags::Get().Player_Block_CursorTrace))
	{
		UnHighlightActor(LastActor);
		UnHighlightActor(ThisActor);
		if (IsValid(ThisActor) && ThisActor->Implements<UHighlightInterface>())
		{

			LastActor = nullptr;
			ThisActor = nullptr;
			return;
		}

	}
	const ECollisionChannel TraceChannel = IsValid(SkillCircle) ? ECC_ExcludePlayers : ECC_Visibility;
	GetHitResultUnderCursor(TraceChannel, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;

	LastActor = ThisActor;
	//ThisActor = Cast<IHighlightInterface>(CursorHit.GetActor());
	// Cursor�� ����Ű�� ���� ��ȿ + Interface�� ĳ���� ����?
	if (IsValid(CursorHit.GetActor()) && CursorHit.GetActor()->Implements<UHighlightInterface>())
	{
		ThisActor = CursorHit.GetActor();
	}
	else
	{
		ThisActor = nullptr;
	}

	/**
	* Line trace from cursor. There are several scenarios:
	*  A. LastActor is null && ThisActor is null
	*		- Do nothing
	*	B. LastActor is null && ThisActor is valid
	*		- Highlight ThisActor
	*	C. LastActor is valid && ThisActor is null
	*		- UnHighlight LastActor
	*	D. Both actors are valid, but LastActor != ThisActor
	*		- UnHighlight LastActor, and Highlight ThisActor
	*	E. Both actors are valid, and are the same actor
	*		- Do nothing
	*/

	// 
	if(LastActor != ThisActor) // LastActor is valid
	{
		// LastActor�� UnHighLight / ThisActor�� HighLight
		UnHighlightActor(LastActor);
		HighlightActor(ThisActor);
	}
}

void ADreamPlayerController::HighlightActor(AActor* InActor)
{
	if (IsValid(InActor) && InActor->Implements<UHighlightInterface>())
	{
		IHighlightInterface::Execute_HighlightActor(InActor);
	}
}

void ADreamPlayerController::UnHighlightActor(AActor* InActor)
{
	if (IsValid(InActor) && InActor->Implements<UHighlightInterface>())
	{
		IHighlightInterface::Execute_UnHighlightActor(InActor);
	}
}

void ADreamPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FDreamGameplayTags::Get().Player_Block_InputPressed))
	{
		return;
	}
	if (InputTag.MatchesTagExact(FDreamGameplayTags::Get().InputTag_RMB))
	{
		if (IsValid(ThisActor))
		{
			// EnemyInterface�� ĳ���� ����? -> �� or ���ƴ�
			TargetingStatus = ThisActor->Implements<UEnemyInterface>() ? ETargetingStatus::TargetingEnemy : ETargetingStatus::TargetingNonEnemy;;
		}
		else
		{
			// ThisActor�� Invalid �� ��� NonTargeting
			TargetingStatus = ETargetingStatus::NotTargeting;
		}
		bAutoRunning = false;
	}
	if (GetASC()) GetASC()->AbilityInputTagPressed(InputTag);
}

void ADreamPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FDreamGameplayTags::Get().Player_Block_InputReleased))
	{
		return;
	}
	if (!InputTag.MatchesTagExact(FDreamGameplayTags::Get().InputTag_RMB))
	{
		if (GetASC()) GetASC()->AbilityInputTagReleased(InputTag);
		return;
	}

	if (GetASC()) GetASC()->AbilityInputTagReleased(InputTag);

	// ���� Ÿ���� X -> �ڵ��̵�
	if (TargetingStatus != ETargetingStatus::TargetingEnemy)
	{
		const APawn* ControlledPawn = GetPawn();
		if (FollowTime <= ShortPressThreshold && ControlledPawn)
		{
			// ���Ϳ� Ŀ���� ������ ���� ��������
			// Valid Actor�� ���̶���Ʈ ȿ���� ������ �� ������? -> ����
			if (IsValid(ThisActor) && ThisActor->Implements<UHighlightInterface>())
			{
				IHighlightInterface::Execute_SetMoveToLocation(ThisActor, CachedDestination);
			}
			else if (GetASC() && !GetASC()->HasMatchingGameplayTag(FDreamGameplayTags::Get().Player_Block_InputPressed))
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ClickNiagaraSystem, CachedDestination);
			}
			if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination))
			{
				Spline->ClearSplinePoints();
				for (const FVector& PointLoc : NavPath->PathPoints)
				{
					Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
				}
				// ��ǥ����(Num())�� ��ó(Num()-1_�� �̵��ϰԲ�
				if (NavPath->PathPoints.Num() > 0)
				{
					CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
					bAutoRunning = true;
				}
			}
		}
		FollowTime = 0.f;
		TargetingStatus = ETargetingStatus::NotTargeting;
	}
}

void ADreamPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FDreamGameplayTags::Get().Player_Block_InputHeld))
	{
		return;
	}
	// InputTag�� ������ ���콺(RMB) ��ư�� �ƴ϶�� return
	if (!InputTag.MatchesTagExact(FDreamGameplayTags::Get().InputTag_RMB))
	{
		if (GetASC())
		{
			// �ش� InputTag�� ���ȴٰ� �˸�.
			GetASC()->AbilityInputTagHeld(InputTag);
		}
		return;
	}

	// ���� Ÿ���� �ϰų� ShiftŰ�� ���� ��� 
	if (TargetingStatus == ETargetingStatus::TargetingEnemy || bShiftKeyDown)
	{
		if (GetASC())
		{
			// �ش� InputTag�� ���ȴٰ� �˸�.
			GetASC()->AbilityInputTagHeld(InputTag);
		}
	}
	else
	{
		FollowTime += GetWorld()->GetDeltaSeconds();

		if (CursorHit.bBlockingHit)
		{
			CachedDestination = CursorHit.ImpactPoint;
		}

		if (APawn* ControlledPawn = GetPawn())
		{
			const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
			ControlledPawn->AddMovementInput(WorldDirection);
		}
	}
}

UDreamAbilitySystemComponent* ADreamPlayerController::GetASC()
{
	if (DreamAbilitySystemComponent == nullptr)
	{
		DreamAbilitySystemComponent = Cast<UDreamAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return DreamAbilitySystemComponent;
}

void ADreamPlayerController::AutoRun()
{
	if (!bAutoRunning) return;

	if (APawn* ControlledPawn = GetPawn())
	{
		// ��ǥ �������� ���� Spline ���󿡼� ���� ��ġ�� ������
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World);
		// ���� �������� ��ǥ���������� ������ ������ �̵�.
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World);
		ControlledPawn->AddMovementInput(Direction);

		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
		if (DistanceToDestination <= AutoRunAcceptanceRadius)
		{
			bAutoRunning = false;
		}
	}
}

void ADreamPlayerController::UpdateMagicCircleLocation()
{
	if (IsValid(SkillCircle))
	{
		SkillCircle->SetActorLocation(CursorHit.ImpactPoint);
	}
}
