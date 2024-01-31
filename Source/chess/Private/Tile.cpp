// Fill out your copyright notice in the Description page of Project Settings.


#include "Tile.h"

// Sets default values
ATile::ATile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// template function which creates a component
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));

	SetRootComponent(Scene);
	StaticMeshComponent->SetupAttachment(Scene);

	Id = TEXT("");
	Status = ETileStatus::EMPTY;
	PlayerOwner = -1;
	TileGridPosition = FVector2D(0, 0);

}

void ATile::SetId(const FString TileId)
{
	Id = TileId;
}

FString ATile::GetId() const
{
	return Id;
}

void ATile::SetTileStatus(const int32 TileOwner, const ETileStatus TileStatus)
{
	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("SETTO TILE CON %d %d"), TileOwner, TileStatus));
	PlayerOwner = TileOwner;
	Status = TileStatus;
}

ETileStatus ATile::GetTileStatus() const
{
	return Status;
}

int32 ATile::GetOwner() const
{
	return PlayerOwner;
}

void ATile::SetGridPosition(const double InX, const double InY)
{
	TileGridPosition.Set(InX, InY);
}

FVector2D ATile::GetGridPosition() const
{
	return TileGridPosition;
}

// Called when the game starts or when spawned
void ATile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
/* void ATile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

} */

