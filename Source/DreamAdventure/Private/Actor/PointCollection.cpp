// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/PointCollection.h"

#include "AbilitySystem/DreamAbilitySystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

APointCollection::APointCollection()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Point_0 = CreateDefaultSubobject<USceneComponent>("Point_0");
	ImmutablePoints.Add(Point_0);
	SetRootComponent(Point_0);

	Point_1 = CreateDefaultSubobject<USceneComponent>("Point_1");
	ImmutablePoints.Add(Point_1);
	Point_1->SetupAttachment(GetRootComponent());

	Point_2 = CreateDefaultSubobject<USceneComponent>("Point_2");
	ImmutablePoints.Add(Point_2);
	Point_2->SetupAttachment(GetRootComponent());

	Point_3 = CreateDefaultSubobject<USceneComponent>("Point_3");
	ImmutablePoints.Add(Point_3);
	Point_3->SetupAttachment(GetRootComponent());

	Point_4 = CreateDefaultSubobject<USceneComponent>("Point_4");
	ImmutablePoints.Add(Point_4);
	Point_4->SetupAttachment(GetRootComponent());

}

TArray<USceneComponent*> APointCollection::GetGroundPoints(const FVector& GroundLocation, int32 NumPoints, float YawOverride)
{
	checkf(ImmutablePoints.Num() >= NumPoints, TEXT("Attempted to access ImmutablePoints out of bounds."));

	TArray<USceneComponent*> ArrayCopy;
	for (USceneComponent* Point : ImmutablePoints)
	{
		// ArrayCopy가 NumPoints를 충족하게 되면 그때 ArrayCopy return
		if (ArrayCopy.Num() >= NumPoints)	return ArrayCopy;

		if (Point != Point_0)
		{
			// Center(Point_0 -> Point_1,2,3...)
			FVector ToPoint = Point->GetComponentLocation() - Point_0->GetComponentLocation();
			// Yaw Override로 회전.
			ToPoint = ToPoint.RotateAngleAxis(YawOverride, FVector::UpVector);
			// 원점(Point_0) 을 기준으로 Point의 Vector 설정.(World Coordinate)
			Point->SetWorldLocation(Point_0->GetComponentLocation() + ToPoint);
		}
		// 경사진 곳에 Point가 있다면 그것을 고려하기 위한 + 500.f
		FVector RaisedLocation = FVector(Point->GetComponentLocation().X, Point->GetComponentLocation().Y, Point->GetComponentLocation().Z + 500.f);
		FVector LoweredLocation = FVector(Point->GetComponentLocation().X, Point->GetComponentLocation().Y, Point->GetComponentLocation().Z - 500.f);

		FHitResult HitResult;
		TArray<AActor*> IgnoreActors;
		// OVerlapping 된 Actor들을 IgnoreActor로 채워넣고
		UDreamAbilitySystemLibrary::GetLivePlayersWithinRadius(this, IgnoreActors, TArray<AActor*>(), 1500.f, GetActorLocation());

		FCollisionQueryParams QueryParams;
		// Overlapping된 Actor를 무시하기 위해 배열에 추가.
		QueryParams.AddIgnoredActors(IgnoreActors);
		// Start : RaisedLocation , End : LoweredLocation
		GetWorld()->LineTraceSingleByProfile(HitResult, RaisedLocation, LoweredLocation, FName("BlockAll"), QueryParams);

		// LineTraceSingleByProfile에서 Z축방향으로 충돌한 지점의 좌표로 조정(Adjust)
		const FVector AdjustedLocation = FVector(Point->GetComponentLocation().X, Point->GetComponentLocation().Y, HitResult.ImpactPoint.Z);
		Point->SetWorldLocation(AdjustedLocation);	// 조정된 좌표로 포인트 설정.
		Point->SetWorldRotation(UKismetMathLibrary::MakeRotFromZ(HitResult.ImpactNormal));

		// 복사하여 반환할 배열에 Point 추가.
		ArrayCopy.Add(Point);
	}
	return ArrayCopy;
}

void APointCollection::BeginPlay()
{
	Super::BeginPlay();
	
}

