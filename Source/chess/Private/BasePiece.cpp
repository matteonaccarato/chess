// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePiece.h"

// Sets default values
ABasePiece::ABasePiece()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	SetRootComponent(Scene);
	StaticMeshComponent->SetupAttachment(Scene);
}

UStaticMeshComponent* ABasePiece::GetStaticMeshComponent() const { return StaticMeshComponent; }
FString ABasePiece::GetId() const { return Id; }

int ABasePiece::GetPieceNum() const { return PieceNum; }
void ABasePiece::SetPieceNum(int Num) { PieceNum = Num; }

void ABasePiece::SetMaxNumberSteps(int NumberSteps) { MaxNumberSteps = NumberSteps; }
int ABasePiece::GetMaxNumberSteps() const { return MaxNumberSteps; }

void ABasePiece::SetColor(EPieceColor PawnColor) { Color = PawnColor; }
EPieceColor ABasePiece::GetColor() const { return Color; }

TArray<ECardinalDirection> ABasePiece::GetCardinalDirections() const { return CardinalDirections; }

void ABasePiece::SetType(EPieceType PawnType) { Type = PawnType; }
EPieceType ABasePiece::GetType() const { return Type; }

void ABasePiece::SetStatus(EPieceStatus PawnStatus) { Status = PawnStatus; }
EPieceStatus ABasePiece::GetStatus() const { return Status; }

void ABasePiece::SetGridPosition(const double InX, const double InY) { TileGridPosition.Set(InX, InY); }
FVector2D ABasePiece::GetGridPosition() const { return TileGridPosition; }


/*
 * Function: GetXYOffset
 * ----------------------------
 * Compute the offsets (X,Y) based on the paramaters (steps, direction and color)
 *
 * @param Steps			const int8				Number of steps to perform
 * @param Direction		ECardinalDirection		Direction to follow during the move
 * @param PieceColor	EPieceColor				Color of the piece
 *
 * @return				std::pair<int8, int8>	Pair containing XOffset as first argument and YOffset as second one
 */
std::pair<int8, int8> ABasePiece::GetXYOffset(const int8 Steps, const ECardinalDirection Direction) const
{
	// Flag to determine if going to north or south / east or west / ... / and exploit symmetry
	int8 FlagDirection = 0;
	int8 XOffset = 0, YOffset = 0;

	switch (Direction)
	{
	case ECardinalDirection::NORTH:
		FlagDirection = 1;
	case ECardinalDirection::SOUTH:
		FlagDirection = FlagDirection ? FlagDirection : -1;
		XOffset = Steps * FlagDirection;
		YOffset = 0;
		break;

	case ECardinalDirection::NORTHEAST:
		FlagDirection = 1;
	case ECardinalDirection::SOUTHWEST:
		FlagDirection = FlagDirection ? FlagDirection : -1;
		XOffset = Steps * FlagDirection;
		YOffset = Steps * FlagDirection;
		break;

	case ECardinalDirection::EAST:
		FlagDirection = 1;
	case ECardinalDirection::WEST:
		FlagDirection = FlagDirection ? FlagDirection : -1;
		XOffset = 0;
		YOffset = Steps * FlagDirection;
		break;

	case ECardinalDirection::NORTHWEST:
		FlagDirection = 1;
	case ECardinalDirection::SOUTHEAST:
		FlagDirection = FlagDirection ? FlagDirection : -1;
		XOffset = Steps * FlagDirection;
		YOffset = Steps * (-FlagDirection);
		break;

	case ECardinalDirection::KNIGHT_TL:
		YOffset = -1;
	case ECardinalDirection::KNIGHT_TR:
		XOffset = 2;
		YOffset = YOffset ? YOffset : 1;
		break;

	case ECardinalDirection::KNIGHT_RT:
		XOffset = 1;
	case ECardinalDirection::KNIGHT_RB:
		XOffset = XOffset ? XOffset : -1;
		YOffset = 2;
		break;

	case ECardinalDirection::KNIGHT_BR:
		YOffset = 1;
	case ECardinalDirection::KNIGHT_BL:
		XOffset = -2;
		YOffset = YOffset ? YOffset : -1;
		break;

	case ECardinalDirection::KNIGHT_LT:
		XOffset = 1;
	case ECardinalDirection::KNIGHT_LB:
		XOffset = XOffset ? XOffset : -1;
		YOffset = -2;
		break;

	}

	// If a piece should go forward (NORTH), 
	//	if it white, the X should be increased (X = 3; NewX = 4),
	//	otherwise, it should be decreased (X = 3; NewX = 2).
	// To do so, piece color value (casted to int) is used: 
	//	Black => -1
	//	White => +1
	XOffset = XOffset * static_cast<int8>(Color);
	YOffset = YOffset * static_cast<int8>(Color);

	return std::make_pair(XOffset, YOffset);
}


/*
 * Function: CheckDirection
 * ----------------------------
 * Check if the movement along a specific direction is allowed or not: the line is clear and [X,Y] match the constraints of the selected direction.
 * It does not take into account the piece rules, but only the given parameters
 *
 * @param Board				const AGameField*		Board to refer to
 * @param Direction			const EDirection		Direction to do the check on
 * @param NewGridPosition	const FVector2D			X and Y of the new grid position
 * @param CurrGridPosition	const FVector2D			X and Y of the current grid position
 *
 * @return	bool	Whether the movement along the specified direction is allowed or not
 */
bool ABasePiece::CheckDirection(const AGameField* GameBoard, const EDirection Direction, const FVector2D NewGridPosition, const FVector2D CurrGridPosition) const
{
	EPieceColor DirectionFlag = Color;
	int8 DeltaX = (NewGridPosition[0] - CurrGridPosition[0]);
	int8 DeltaY = NewGridPosition[1] - CurrGridPosition[1];
	int8 PawnDiagonalMaxSteps = (Type == EPieceType::PAWN) ? 1 : MaxNumberSteps;

	switch (Direction)
	{
	case EDirection::FORWARD:
		if (DeltaY == 0 && (DeltaX * static_cast<double>(DirectionFlag)) >= 0 && (DeltaX * static_cast<double>(DirectionFlag)) <= MaxNumberSteps)
			return GameBoard->IsLineClear(ELine::VERTICAL, CurrGridPosition, DeltaX, DeltaY);
		break;

	case EDirection::BACKWARD:
		return DeltaY == 0 
			&& ((-DeltaX) * static_cast<double>(DirectionFlag)) >= 0 
			&& ((-DeltaX) * static_cast<double>(DirectionFlag)) <= MaxNumberSteps 
			&& GameBoard->IsLineClear(ELine::VERTICAL, CurrGridPosition, DeltaX, DeltaY);

	case EDirection::HORIZONTAL:
		return DeltaX == 0 
			&& FMath::Abs(DeltaY) >= 0 
			&& FMath::Abs(DeltaY) <= MaxNumberSteps
			&& GameBoard->IsLineClear(ELine::HORIZONTAL, CurrGridPosition, DeltaX, DeltaY);

	case EDirection::DIAGONAL:
		if (FMath::Abs(DeltaX) == FMath::Abs(DeltaY) && FMath::Abs(DeltaX) <= PawnDiagonalMaxSteps)
		{
			if (!GameBoard->IsLineClear(ELine::DIAGONAL, CurrGridPosition, DeltaX, DeltaY))
				return false;

			if (Type == EPieceType::PAWN)
				if (DeltaX * static_cast<int>(Color) < 0)
					return false;

			return true;
		}
		break;

	case EDirection::KNIGHT:
		return (FMath::Abs(DeltaX) == 1 && FMath::Abs(DeltaY) == 2) || (FMath::Abs(DeltaX) == 2 && FMath::Abs(DeltaY) == 1);
	}

	return false;
}


/*
 * Function: Move
 * ----------------------------
 * Make the move specified through paramters
 *
 * @param OldTile	ATile*			Tile where the piece starts from
 * @param NewTile	ATile*			Tile where the piece moves to
 * @param Simulate	bool = false	Determine if the move is just a simulation or not.
 *									If so, graphically moving the piece is not required
 */
void ABasePiece::Move(ATile* OldTile, ATile* NewTile, bool Simulate)
{
	if (OldTile && NewTile)
	{
		OldTile->ClearInfo();

		int8 PlayerOwner = GetColor() == EPieceColor::WHITE ? 0 : 1;
		NewTile->SetPlayerOwner(PlayerOwner);
		NewTile->SetTileStatus({ 
			this, 
			0,
			{ 0, 0 },
			NewTile->GetTileStatus().WhoCanGo, 
			Color, 
			Type,
			PlayerOwner 
		});

		SetGridPosition(NewTile->GetGridPosition()[0], NewTile->GetGridPosition()[1]);
		if (!Simulate)
		{
			FVector SpawnPosition = NewTile->GetActorLocation() + FVector(0, 0, GetActorLocation()[2]);
			SetActorLocation(SpawnPosition);
		}
	}
}


// Called when the game starts or when spawned
void ABasePiece::BeginPlay()
{
	Super::BeginPlay();	
}

// Called every frame
/* void ABasePiece::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
} */

void ABasePiece::SelfDestroy()
{
	Destroy();
}

