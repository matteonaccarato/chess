// Fill out your copyright notice in the Description page of Project Settings.


#include "GameField.h"

// Sets default values
AGameField::AGameField()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AGameField::OnConstruction(const FTransform& Transform)
{
}

// Called when the game starts or when spawned
void AGameField::BeginPlay()
{
	Super::BeginPlay();
	
}

void AGameField::ResetField()
{
}

void AGameField::GenerateField()
{
}

FVector2D AGameField::GetPosition(const FHitResult& Hit)
{
	return FVector2D();
}

TArray<ATile*>& AGameField::GetTileArray()
{
	// TODO: insert return statement here
}

FVector AGameField::GetRelativeLocationByXYPosition(const int32 InX, const int32 InY) const
{
	return FVector();
}

FVector2D AGameField::GetXYPositionByRelativeLocation(const FVector& Location) const
{
	return FVector2D();
}

TArray<int32> AGameField::GetLine(const FVector2D Begin, const FVector2D End) const
{
	return TArray<int32>();
}

// Called every frame
/* void AGameField::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

} */

