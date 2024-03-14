// Fill out your copyright notice in the Description page of Project Settings.


#include "GameField.h"
#include "Kismet/GameplayStatics.h"
#include <string>


// Sets default values
AGameField::AGameField()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	WinSize = 3;

	// Size of the field (8x8)
	Size = 8; 
	TileSize = 120;
	CellPadding = 0;
	Pawns_Rows = 2;
	// NormalizedCellPadding = FMath::RoundToDouble(((TileSize + CellPadding) / TileSize) * 100) / 100;
}

void AGameField::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	NormalizedCellPadding = FMath::RoundToDouble(((TileSize + CellPadding) / TileSize) * 100) / 100;
}

// Called when the game starts or when spawned
void AGameField::BeginPlay()
{
	Super::BeginPlay();
	GenerateField();
}

void AGameField::ResetField()
{
	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		// clear tile status
		for (ATile* Obj : TileArray)
			DespawnPawn(Obj->GetGridPosition()[0], Obj->GetGridPosition()[1]);
		
		// load initial board
		TArray<FTileSaving> InitialBoard;
		int n = PawnArray.Num();
		for (int i = 0; i < n; i++)
		{
			// initial 32 chess pieces
			if (i < 32)
			{
				int8 x = (PawnArray[i]->GetColor() == EPawnColor::WHITE) ?
					PawnArray[i]->GetPieceNum() / 8
					: PawnArray[i]->GetPieceNum() / 8 + 4;
				int8 y = PawnArray[i]->GetPieceNum() % 8;
				PawnArray[i]->SetGridPosition(x, y);
				PawnArray[i]->SetStatus(EPawnStatus::ALIVE);

				PawnArray[i]->Move(TileArray[x * Size + y], TileArray[x * Size + y]);
				if (PawnArray[i]->GetType() == EPawnType::PAWN)
				{
					PawnArray[i]->SetMaxNumberSteps(2); // restore two steps of first move on pawn
				}

				InitialBoard.Add({
					x,
					y,
					EPawnStatus::ALIVE
					});
			}
			else
			{
				// pieces added during the game
				// destroy actor and reference to it
				PawnArray[i]->SelfDestroy();
				PawnArray.RemoveAt(i);
			}
		}
		GameMode->GameSaving.Empty();
		GameMode->CastlingInfoWhite = { false, { false, false } };
		GameMode->CastlingInfoBlack = { false, { false, false } };

		LoadBoard(InitialBoard);


		// clear replay
		if (GameMode->ReplayWidget)
		{
			UScrollBox* ScrollBoxWhite = Cast<UScrollBox>(GameMode->ReplayWidget->GetWidgetFromName(TEXT("scr_Replay_white")));
			ScrollBoxWhite->ClearChildren();
			UScrollBox* ScrollBoxBlack = Cast<UScrollBox>(GameMode->ReplayWidget->GetWidgetFromName(TEXT("scr_Replay_black")));
			ScrollBoxBlack->ClearChildren();
		}
	
		// OnResetEvent.Broadcast();

		GameMode->CheckFlag = EPawnColor::NONE;
		GameMode->CheckMateFlag = EPawnColor::NONE;
		GameMode->IsGameOver = false;
		GameMode->MoveCounter = 0;
		GameMode->ChoosePlayerAndStartGame();
	}
}

void AGameField::GenerateField()
{
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ATile> TileClass;
	TSubclassOf<ABasePawn> BasePawnClass;

	UWorld* World = GetWorld();
	bool flag = false;
	int8 PieceIdx = 0;
	/* TSubclassOf<AActor> Letters[] = {LetterA};
	TSubclassOf<AActor> Numbers[] = { LetterA }; */
	UMaterialInterface* Letters[] = { LetterA, LetterB, LetterC, LetterD, LetterE, LetterF, LetterG, LetterH };
	UMaterialInterface* Numbers[] = { Number1, Number2, Number3, Number4, Number5, Number6, Number7, Number8 };

	for (int32 x = 0; x < Size; x++)
	{
		for (int32 y = 0; y < Size; y++)
		{
			TileClass = flag ? W_TileClass : B_TileClass;

			FVector Location = AGameField::GetRelativeLocationByXYPosition(x, y);
			ATile* TileObj = GetWorld()->SpawnActor<ATile>(TileClass, Location, FRotator::ZeroRotator);
						
			if (TileObj != nullptr)
			{
				// [A-H][1-8]
				const int32 IdChar = 65 + y;
				const int32 IdNum = x + 1;
				const float TileScale = TileSize / 100;

				TileObj->SetLetterId(FString::Printf(TEXT("%c"), IdChar));
				TileObj->SetNumberId(IdNum);

				// Bottom (Letters)
				if (x == 0 && World)
				{
					AActor* Letter = GetWorld()->SpawnActor<AActor>(LetterNumberClass, Location + FVector(-120, 0, 0), FRotator(0, 90, 0));
					if (Letter)
					{
						Letter->SetActorScale3D(FVector(TileScale*0.7, TileScale*0.7, 1));
						UStaticMeshComponent* MeshComponent = Letter->FindComponentByClass<UStaticMeshComponent>();
						if (MeshComponent)
							MeshComponent->SetMaterial(0, Letters[y]);
					}
				}
				
				// Left (Numbers)
				if (y == 0 && World)
				{
					AActor* Number = GetWorld()->SpawnActor<AActor>(LetterNumberClass, Location + FVector(0, -120, 0), FRotator(0, 90, 0));
					if (Number)
					{
						Number->SetActorScale3D(FVector(TileScale*0.7, TileScale*0.7, 1));
						UStaticMeshComponent* MeshComponent = Number->FindComponentByClass<UStaticMeshComponent>();
						if (MeshComponent)
							MeshComponent->SetMaterial(0, Numbers[x]);
					}
				}

				TileObj->SetActorScale3D(FVector(TileScale, TileScale, 0.2));
				TileObj->SetGridPosition(x, y);
				TileArray.Add(TileObj);
				TileMap.Add(FVector2D(x, y), TileObj);

				EPawnType PawnTypesOnRow[] = { EPawnType::ROOK, EPawnType::KNIGHT, EPawnType::BISHOP, EPawnType::QUEEN, EPawnType::KING, EPawnType::BISHOP, EPawnType::KNIGHT, EPawnType::ROOK };
				if (x < Pawns_Rows || (Size - x - 1) < Pawns_Rows)
				{
					if (x == 0)
						SpawnPawn(PawnTypesOnRow[y], EPawnColor::WHITE, x, y, 0);
					else if (x == 1)
						SpawnPawn(EPawnType::PAWN, EPawnColor::WHITE, x, y, 0);
					else if (x == Size - 1)
						SpawnPawn(PawnTypesOnRow[y], EPawnColor::BLACK, x, y, 1);
					else
						SpawnPawn(EPawnType::PAWN, EPawnColor::BLACK, x, y, 1);
				}
				flag = !flag;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("ATile Obj is null"));
			}
		}
		flag = !flag;
	}
}


FVector2D AGameField::GetPosition(const FHitResult& Hit) { return Cast<ATile>(Hit.GetActor())->GetGridPosition(); }
TArray<ATile*>& AGameField::GetTileArray() { return TileArray; }
TArray<ABasePawn*>& AGameField::GetPawnArray() { return PawnArray; }

FVector AGameField::GetRelativeLocationByXYPosition(const int32 InX, const int32 InY) const
{
	return TileSize * NormalizedCellPadding * FVector(InX, InY, 0);
}

FVector2D AGameField::GetXYPositionByRelativeLocation(const FVector& Location) const
{
	const double x = Location[0] / (TileSize * NormalizedCellPadding);
	const double y = Location[1] / (TileSize * NormalizedCellPadding);

	return FVector2D(x, y);
}

void AGameField::LoadBoard(const TArray<FTileSaving>& Board)
{
	// memorizzo in ordine come pawnarray [0..32] e ottengo tile X Y | for()
	// in load board
	//		for (pawnarray)	pawn->setactorlocation()

	FVector Origin, BoxExtent, Location, PawnLocation;
	for (int8 i = 0; i < PawnArray.Num(); i++)
	{
		if (Board.IsValidIndex(i))
		{
			switch (Board[i].Status)
			{
			case EPawnStatus::ALIVE:
				PawnArray[i]->SetActorHiddenInGame(false);
				PawnArray[i]->SetActorEnableCollision(true);
				PawnArray[i]->SetActorTickEnabled(true);

				TileArray[Board[i].X * Size + Board[i].Y]->GetActorBounds(false, Origin, BoxExtent);
				Location = GetRelativeLocationByXYPosition(Board[i].X, Board[i].Y);
				PawnLocation = FVector(
					Location.GetComponentForAxis(EAxis::X),
					Location.GetComponentForAxis(EAxis::Y),
					Location.GetComponentForAxis(EAxis::Z) + 2 * BoxExtent.GetComponentForAxis(EAxis::Z) + 0.1
				);
				PawnArray[i]->SetActorLocation(PawnLocation);
				break;

			case EPawnStatus::DEAD:
				PawnArray[i]->SetActorHiddenInGame(true);
				PawnArray[i]->SetActorEnableCollision(false);
				PawnArray[i]->SetActorTickEnabled(false);
				PawnArray[i]->SetActorLocation(GetRelativeLocationByXYPosition(PawnArray[i]->GetGridPosition()[0], PawnArray[i]->GetGridPosition()[0]) + FVector(0, 0, -20));
				break;
			}
		}
		else 
		{
			PawnArray[i]->SetActorHiddenInGame(true);
			PawnArray[i]->SetActorEnableCollision(false);
			PawnArray[i]->SetActorTickEnabled(false);
			PawnArray[i]->SetActorLocation(GetRelativeLocationByXYPosition(PawnArray[i]->GetGridPosition()[0], PawnArray[i]->GetGridPosition()[0]) + FVector(0, 0, -20));
		}
	}
}




bool AGameField::CheckDirection(const EDirection Direction, ABasePawn* Pawn, const FVector2D NewGridPosition, const FVector2D CurrGridPosition, const bool TestFlag) const
{
	EPawnColor DirectionFlag = Pawn->GetColor();
	int8 DeltaX = (NewGridPosition[0] - CurrGridPosition[0]); // * static_cast<double>(DirectionFlag);
	int8 DeltaY = NewGridPosition[1] - CurrGridPosition[1];
	int8 MaxSteps = (Pawn->GetType() == EPawnType::PAWN) ? 1 : Pawn->GetMaxNumberSteps();

	switch (Direction)
	{
	case EDirection::FORWARD:
		if (DeltaY == 0 && (DeltaX * static_cast<double>(DirectionFlag)) >= 0 && (DeltaX * static_cast<double>(DirectionFlag)) <= Pawn->GetMaxNumberSteps())
		{
			if (!IsLineClear(ELine::VERTICAL, CurrGridPosition, DeltaX, DeltaY))
				return false;

			if (Pawn->GetType() == EPawnType::PAWN && !TestFlag)
				Pawn->SetMaxNumberSteps(1);

			return true;
		}
		break;

	case EDirection::BACKWARD:
		return DeltaY == 0 && ((-DeltaX) * static_cast<double>(DirectionFlag)) >= 0 && ((-DeltaX) * static_cast<double>(DirectionFlag)) <= Pawn->GetMaxNumberSteps() && IsLineClear(ELine::VERTICAL, CurrGridPosition, DeltaX, DeltaY);

	case EDirection::HORIZONTAL:
		return DeltaX == 0 && FMath::Abs(DeltaY) >= 0 && FMath::Abs(DeltaY) <= Pawn->GetMaxNumberSteps() && IsLineClear(ELine::HORIZONTAL, CurrGridPosition, DeltaX, DeltaY);

	case EDirection::DIAGONAL:
		if (FMath::Abs(DeltaX) == FMath::Abs(DeltaY) && FMath::Abs(DeltaX) <= MaxSteps)
		{
			if (!IsLineClear(ELine::DIAGONAL, CurrGridPosition, DeltaX, DeltaY))
				return false;

			if (Pawn->GetType() == EPawnType::PAWN)
				if (DeltaX * static_cast<int>(Pawn->GetColor()) < 0)
					return false;

			if (Pawn->GetType() == EPawnType::PAWN && !TestFlag)
				Pawn->SetMaxNumberSteps(1);

			return true;
		}
		break;

	case EDirection::KNIGHT:
		return (FMath::Abs(DeltaX) == 1 && FMath::Abs(DeltaY) == 2) || (FMath::Abs(DeltaX) == 2 && FMath::Abs(DeltaY) == 1);
	}

	return false;
}






/*
* return false if there is a pawn along the movement
*/
bool AGameField::IsLineClear(ELine Line, const FVector2D CurrGridPosition, const int8 DeltaX, const int8 DeltaY) const
{
	switch (Line)
	{
	case ELine::HORIZONTAL:
		for (int8 YOffset = 1; YOffset < FMath::Abs(DeltaY); YOffset++)
		{
			if (IsValidTile(CurrGridPosition[0], CurrGridPosition[1] + YOffset * FMath::Sign(DeltaY))
				&& !TileArray[CurrGridPosition[0] * Size + CurrGridPosition[1] + YOffset * FMath::Sign(DeltaY)]->GetTileStatus().EmptyFlag)
				return false;
		}
		break;

	case ELine::VERTICAL:
		for (int8 XOffset = 1; XOffset < FMath::Abs(DeltaX); XOffset++)
		{
			if (IsValidTile(CurrGridPosition[0] + XOffset * FMath::Sign(DeltaX), CurrGridPosition[1])
				&& !TileArray[(CurrGridPosition[0] + XOffset * FMath::Sign(DeltaX)) * Size + CurrGridPosition[1]]->GetTileStatus().EmptyFlag)
				return false;
		}
		break;

	case ELine::DIAGONAL:
		for (int8 Offset = 1; Offset < FMath::Abs(DeltaX); Offset++)
		{
			if (IsValidTile(CurrGridPosition[0] + Offset * FMath::Sign(DeltaX), CurrGridPosition[1] + Offset * FMath::Sign(DeltaY))
				&& !TileArray[(CurrGridPosition[0] + Offset * FMath::Sign(DeltaX)) * Size + CurrGridPosition[1] + Offset * FMath::Sign(DeltaY)]->GetTileStatus().EmptyFlag)
				return false;
		}
		break;
	}

	return true;
}


bool AGameField::IsValidTile(const int8 X, const int8 Y) const
{
	return X >= 0 && X < Size
		&& Y >= 0 && Y < Size;
}


/*
 * Function: SpawnPawn
 * ----------------------------
 *   Spawn the pawn specified through parameters
 *
 *	 PawnType	EPawnType	: type of the pawn to spawn
 *   PawnColor	EPawnColor	: color of the pawn to spawn
 *   X			int8		: x position of the pawn to spawn
 *	 Y			int8		: y position of the pawn to spawn
 *
 *   return: Pointer to the recently spawned pawn
 */
ABasePawn* AGameField::SpawnPawn(EPawnType PawnType, EPawnColor PawnColor, int8 X, int8 Y, int8 PlayerOwner)
{
	// TODO => necessario fare if (PawnType && PawnColor && X && Y) essendo parametri obbligatori ??

	TSubclassOf<ABasePawn> BasePawnClass;
	ABasePawn* BasePawnObj = nullptr;

	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode && IsValidTile(X, Y))
	{
		ATile* TileObj = GetTileArray()[X * Size + Y];
		TileObj->SetPlayerOwner((PlayerOwner != -1) ? PlayerOwner : GameMode->CurrentPlayer);
		TArray<bool> TmpFalse; TmpFalse.Add(false); TmpFalse.Add(false);
		FTileStatus TileStatus = { 0, TmpFalse, TileObj->GetTileStatus().WhoCanGo, EPawnColor::NONE, EPawnType::NONE };

		TSubclassOf<ABasePawn> W_PawnsClasses[] = { W_RookClass, W_KnightClass, W_BishopClass, W_QueenClass, W_KingClass, W_PawnClass };
		TSubclassOf<ABasePawn> B_PawnsClasses[] = { B_RookClass, B_KnightClass, B_BishopClass, B_QueenClass, B_KingClass, B_PawnClass };

		// Set the pawn type in the tile status 
		switch (PawnType)
		{
		case EPawnType::ROOK: TileStatus.PawnType = EPawnType::ROOK; break;
		case EPawnType::KNIGHT: TileStatus.PawnType = EPawnType::KNIGHT; break;
		case EPawnType::BISHOP: TileStatus.PawnType = EPawnType::BISHOP; break;
		case EPawnType::QUEEN: TileStatus.PawnType = EPawnType::QUEEN; break;
		case EPawnType::KING: TileStatus.PawnType = EPawnType::KING; break;
		case EPawnType::PAWN: TileStatus.PawnType = EPawnType::PAWN; break;
		}

		// Choose correct class (White or Black)
		switch (PawnColor)
		{
		case EPawnColor::WHITE:
			TileStatus.PawnColor = EPawnColor::WHITE;
			BasePawnClass = W_PawnsClasses[static_cast<int>(PawnType) - 1];
			break;
		case EPawnColor::BLACK:
			TileStatus.PawnColor = EPawnColor::BLACK;
			BasePawnClass = B_PawnsClasses[static_cast<int>(PawnType) - 1];
			break;
		}

		// Calculate spawn location
		FVector Origin; FVector BoxExtent;
		TileObj->GetActorBounds(false, Origin, BoxExtent);
		FVector Location = GetRelativeLocationByXYPosition(X, Y);
		FVector PawnLocation(
			Location.GetComponentForAxis(EAxis::X),
			Location.GetComponentForAxis(EAxis::Y),
			Location.GetComponentForAxis(EAxis::Z) + 2 * BoxExtent.GetComponentForAxis(EAxis::Z) + 0.1
		);

		BasePawnObj = GetWorld()->SpawnActor<ABasePawn>(BasePawnClass, PawnLocation, FRotator(0, 90, 0));
		if (BasePawnObj != nullptr)
		{
			BasePawnObj->SetGridPosition(X, Y);
			// TODO: 0.8 da mettere come attributo
			const float TileScale = TileSize / 100;
			BasePawnObj->SetActorScale3D(FVector(TileScale * 0.8, TileScale * 0.8, 0.03));			
			BasePawnObj->SetPieceNum(PawnArray.Num());
			BasePawnObj->SetType(TileStatus.PawnType);
			BasePawnObj->SetColor(TileStatus.PawnColor);
			BasePawnObj->SetStatus(EPawnStatus::ALIVE);

			PawnArray.Add(BasePawnObj);
			PawnMap.Add(FVector2D(X, Y), BasePawnObj);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ABasePawn Obj is null"));
		}

		TileObj->SetPlayerOwner(PlayerOwner);
		TileObj->SetTileStatus(TileStatus);
		TileObj->SetPawn(BasePawnObj);
	}
	return BasePawnObj;
}

/*
 * Function: DespawnPawn
 * ----------------------------
 *   Despawn pawn which is on the tile specified
 *
 *   X			int8		: x position of the pawn to despawn
 *	 Y			int8		: y position of the pawn to despawn
 */
void AGameField::DespawnPawn(int8 X, int8 Y)
{
	if (IsValidTile(X, Y))
	{
		ATile* Tile = GetTileArray()[X * Size + Y];
		ABasePawn* Pawn = Tile->GetPawn();
		if (Tile)
		{
			// Reset old tile status
			Tile->SetPawn(nullptr);
			TArray<bool> TmpFalse; TmpFalse.Add(false); TmpFalse.Add(false);
			Tile->SetTileStatus({ 1, TmpFalse, Tile->GetTileStatus().WhoCanGo, EPawnColor::NONE, EPawnType::NONE });
			Tile->SetPlayerOwner(ChessEnums::NOT_ASSIGNED);
		}

		if (Pawn)
		{
			// Update pawn information
			Pawn->SetStatus(EPawnStatus::DEAD);
			Pawn->SetGridPosition(-1, -1);
			Pawn->SetActorHiddenInGame(true);
			Pawn->SetActorEnableCollision(false);
			Pawn->SetActorTickEnabled(false);
			GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Blue, FString::Printf(TEXT("%f %f pawn has been eaten/despawned"), Pawn->GetGridPosition()[0], Pawn->GetGridPosition()[1]));
		}
	}
}



// Called every frame
/* void AGameField::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

} */