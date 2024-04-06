// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePawn.h"

// Sets default values
ABasePawn::ABasePawn()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// TODO: forse da settare su true per movimento dinamico (come cono)

}

FString ABasePawn::GetId() const
{
	return Id;
}

int ABasePawn::GetPieceNum() const
{
	return PieceNum;
}

void ABasePawn::SetPieceNum(int Num)
{
	PieceNum = Num;
}

void ABasePawn::SetMaxNumberSteps(int32 NumberSteps)
{
	MaxNumberSteps = NumberSteps;
}

int32 ABasePawn::GetMaxNumberSteps() const
{
	return MaxNumberSteps;
}

void ABasePawn::SetColor(EPawnColor PawnColor)
{
	Color = PawnColor;
}


EPawnColor ABasePawn::GetColor() const
{
	return Color;
}

TArray<ECardinalDirection> ABasePawn::GetCardinalDirections() const
{
	return CardinalDirections;
}

void ABasePawn::SetType(EPawnType PawnType)
{
	Type = PawnType;
}

EPawnType ABasePawn::GetType() const
{
	return Type;
}

void ABasePawn::SetStatus(EPawnStatus PawnStatus)
{
	Status = PawnStatus;
}

EPawnStatus ABasePawn::GetStatus() const
{
	return Status;
}

void ABasePawn::SetGridPosition(const double InX, const double InY)
{
	TileGridPosition.Set(InX, InY);
}

FVector2D ABasePawn::GetGridPosition() const
{
	return TileGridPosition;
}

void ABasePawn::Move(ATile* OldTile, ATile* NewTile, bool Simulate)
{
	// TODO => just to test
	if (Type == EPawnType::ROOK && !Simulate)
	{
		Type = Type;
	}
	if (OldTile && NewTile)
	{
		OldTile->ClearInfo();

		int8 PlayerOwner = GetColor() == EPawnColor::WHITE ? 0 : 1;
		NewTile->SetPlayerOwner(PlayerOwner);
		NewTile->SetTileStatus({ this, 0, { 0, 0 }, NewTile->GetTileStatus().WhoCanGo, GetColor(), GetType(), PlayerOwner });
		NewTile->SetPawn(this);

		FVector SpawnPosition = NewTile->GetActorLocation() + FVector(0, 0, GetActorLocation()[2]);
		SetGridPosition(NewTile->GetGridPosition()[0], NewTile->GetGridPosition()[1]);
		if (!Simulate)
			SetActorLocation(SpawnPosition);
	}
}


// Called when the game starts or when spawned
void ABasePawn::BeginPlay()
{
	Super::BeginPlay();


	/* AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode != nullptr)
	{
		// TODO => riposizionare pedina nel punto di partenza
		GameMode->GField->OnResetEvent.AddDynamic(this, &ABasePawn::SelfDestroy);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode is null"));
	} */
	
}

// Called every frame
/* void ABasePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
} */

void ABasePawn::SelfDestroy()
{
	Destroy();
}

