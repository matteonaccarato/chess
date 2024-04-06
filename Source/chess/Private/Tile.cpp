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
	MaterialLight = CreateDefaultSubobject<UMaterialInterface>("MaterialLight");
	MaterialDark = CreateDefaultSubobject<UMaterialInterface>("MaterialDark");
	// TODO => material green ?
	MaterialGreen = CreateDefaultSubobject<UMaterialInterface>("MaterialGreen");

	SetRootComponent(Scene);
	StaticMeshComponent->SetupAttachment(Scene);

	LetterId = TEXT("");
	NumberId = -1;

	Status = { nullptr, 1, {0, 0}, TArray<ABasePawn*>(), EPawnColor::NONE, EPawnType::NONE, ChessEnums::NOT_ASSIGNED};
	PlayerOwner = -1;
	TileGridPosition = FVector2D(0, 0);

}

UStaticMeshComponent* ATile::GetStaticMeshComponent() const
{
	return StaticMeshComponent;
}

void ATile::SetPawn(ABasePawn* TilePawn)
{
	Pawn = TilePawn;
}

ABasePawn* ATile::GetPawn() const
{
	return Pawn;
}

FString ATile::GetId() const
{
	return LetterId + FString::Printf(TEXT("%d"), NumberId);
}

void ATile::SetLetterId(const FString TileLetter)
{
	LetterId = TileLetter;
}

FString ATile::GetLetterId() const
{
	return LetterId;
}

void ATile::SetNumberId(const int8 TileNumber)
{
	NumberId = TileNumber;
}

int8 ATile::GetNumberId() const
{
	return NumberId;
}

void ATile::SetTileStatus(const FTileStatus TileStatus)
{
	Status = TileStatus;
}

FTileStatus ATile::GetTileStatus() const
{
	return Status;
}

void ATile::SetPlayerOwner(const int32 P_Owner)
{
	PlayerOwner = P_Owner;
}

int32 ATile::GetPlayerOwner() const
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

void ATile::ClearInfo()
{
	SetPawn(nullptr);
	SetPlayerOwner(ChessEnums::NOT_ASSIGNED);
	SetTileStatus({ nullptr, 1, {0, 0},  GetTileStatus().WhoCanGo, EPawnColor::NONE, EPawnType::NONE, ChessEnums::NOT_ASSIGNED});
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

