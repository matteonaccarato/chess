// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePawn.h"

// Sets default values
ABasePawn::ABasePawn()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// TODO: forse da settare su true per movimento dinamico (come cono)
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	SetRootComponent(Scene);
	StaticMeshComponent->SetupAttachment(Scene);
}

UStaticMeshComponent* ABasePawn::GetStaticMeshComponent() const
{
	return StaticMeshComponent;
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


bool ABasePawn::CheckDirection(const AGameField* GameBoard, const EDirection Direction, const FVector2D NewGridPosition, const FVector2D CurrGridPosition, const bool TestFlag)
{
	EPawnColor DirectionFlag = GetColor();
	int8 DeltaX = (NewGridPosition[0] - CurrGridPosition[0]);
	int8 DeltaY = NewGridPosition[1] - CurrGridPosition[1];
	int8 MaxSteps = (GetType() == EPawnType::PAWN) ? 1 : GetMaxNumberSteps();

	switch (Direction)
	{
	case EDirection::FORWARD:
		if (DeltaY == 0 && (DeltaX * static_cast<double>(DirectionFlag)) >= 0 && (DeltaX * static_cast<double>(DirectionFlag)) <= GetMaxNumberSteps())
		{
			if (!GameBoard->IsLineClear(ELine::VERTICAL, CurrGridPosition, DeltaX, DeltaY))
				return false;

			if (GetType() == EPawnType::PAWN && !TestFlag)
				SetMaxNumberSteps(1);

			return true;
		}
		break;

	case EDirection::BACKWARD:
		return DeltaY == 0 
			&& ((-DeltaX) * static_cast<double>(DirectionFlag)) >= 0 
			&& ((-DeltaX) * static_cast<double>(DirectionFlag)) <= GetMaxNumberSteps() 
			&& GameBoard->IsLineClear(ELine::VERTICAL, CurrGridPosition, DeltaX, DeltaY);

	case EDirection::HORIZONTAL:
		return DeltaX == 0 
			&& FMath::Abs(DeltaY) >= 0 
			&& FMath::Abs(DeltaY) <= GetMaxNumberSteps()
			&& GameBoard->IsLineClear(ELine::HORIZONTAL, CurrGridPosition, DeltaX, DeltaY);

	case EDirection::DIAGONAL:
		if (FMath::Abs(DeltaX) == FMath::Abs(DeltaY) && FMath::Abs(DeltaX) <= MaxSteps)
		{
			if (!GameBoard->IsLineClear(ELine::DIAGONAL, CurrGridPosition, DeltaX, DeltaY))
				return false;

			if (GetType() == EPawnType::PAWN)
				if (DeltaX * static_cast<int>(GetColor()) < 0)
					return false;

			if (GetType() == EPawnType::PAWN && !TestFlag)
				SetMaxNumberSteps(1);

			return true;
		}
		break;

	case EDirection::KNIGHT:
		return (FMath::Abs(DeltaX) == 1 && FMath::Abs(DeltaY) == 2) || (FMath::Abs(DeltaX) == 2 && FMath::Abs(DeltaY) == 1);
	}

	return false;
}




/*
 * Function: GetXYOffset
 * ----------------------------
 *   Computes the offsets (X,Y) based on the paramaters (steps, direction and color)
 *
 *	 Steps		const int8				number of steps to perform
 *	 Direction	ECardinalDirection		direction to follow during the move
 *	 PieceColor	EPawnColor				color of the piece
 *
 *   return		std::pair<int8, int8>	pair containing XOffset as first and YOffset as second
 */
std::pair<int8, int8> ABasePawn::GetXYOffset(const int8 Steps, const ECardinalDirection Direction) const
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

	// TODO => cast to int8, not int
	// If a piece should go forward (NORTH), 
	//	if it white, the X should be increased (X = 3; NewX = 4),
	//	otherwise, it should be decreased (X = 3; NewX = 2).
	// To do so, piece color value (casted to int) is used: 
	//	Black => -1
	//	White => +1
	XOffset = XOffset * static_cast<int>(Color);
	YOffset = YOffset * static_cast<int>(Color);

	return std::make_pair(XOffset, YOffset);
}



/*
 * Function: Move
 * ----------------------------
 *   It makes the move specified through paramters
 *
 *	 OldTile	ATile*	Tile where the piece starts from
 *	 NewTile	ATile*	Tile where the piece moves to
 *	 Simulate	bool = false	Determines if the move is just a simulation or not.
 *								If so, graphically moving the piece is not required
 */
void ABasePawn::Move(ATile* OldTile, ATile* NewTile, bool Simulate)
{
	if (OldTile && NewTile)
	{
		OldTile->ClearInfo();

		int8 PlayerOwner = GetColor() == EPawnColor::WHITE ? 0 : 1;
		NewTile->SetPlayerOwner(PlayerOwner);
		NewTile->SetTileStatus({ 
			this, 
			0, // TODO => empty flag da fare come enum FEmptyStatus EMPTY | OCCUPIED
			{ 0, 0 },
			NewTile->GetTileStatus().WhoCanGo, 
			GetColor(), 
			GetType(),
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
void ABasePawn::BeginPlay()
{
	Super::BeginPlay();	
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

