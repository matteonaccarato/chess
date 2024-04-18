// Fill out your copyright notice in the Description page of Project Settings.


#include "GameField.h"
#include "Kismet/GameplayStatics.h"
#include <string>


// Sets default values
AGameField::AGameField()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	Size		= GAMEBOARD_SIZE;
	TileSize	= TILE_SIZE;
	TilePadding	= TILE_PADDING;
	Pawns_Rows	= PAWN_ROWS;
}

void AGameField::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	NormalizedCellPadding = FMath::RoundToDouble(((TileSize + TilePadding) / TileSize) * 100) / 100;
}

// Called when the game starts or when spawned
void AGameField::BeginPlay()
{
	Super::BeginPlay();
	GenerateField();
}

/*
 * Function: ResetField
 * ----------------------------
 *   Reset the game board (graphically and resetting data structures)
 *
 *	 bRestartGame	bool	Flag to notify is starting a new game is required or not
 */
void AGameField::ResetField(bool bRestartGame)
{
	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->GameInstance->Minutes = 0;
		GameMode->GameInstance->Seconds = 0;
		GetWorldTimerManager().ClearTimer(GameMode->StopwatchTimerHandle);

		for (int8 i = 0; i < GameMode->Players.Num(); i++)
			GameMode->Players[i]->IsMyTurn = false;

		// clear tile status
		for (ATile* Obj : TileArray)
			DespawnPawn(Obj->GetGridPosition()[0], Obj->GetGridPosition()[1]);
		
		// load initial board
		TArray<FPieceSaving> InitialBoard;
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
				if (PawnArray.IsValidIndex(i))
				{
					PawnArray[i]->SelfDestroy();
					PawnArray.RemoveAt(i);
				}
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
		GameMode->MatchStatus = EMatchResult::NONE;
		GameMode->IsGameOver = false;
		GameMode->MoveCounter = 0;
		GameMode->ReplayInProgress = 0;
		if (bRestartGame)
			GameMode->ChoosePlayerAndStartGame();
	}
}


/*
 * Function: GenerateField
 * ----------------------------
 *   Generate the game board (graphically and initializing data structures)
 */
void AGameField::GenerateField()
{
	UPROPERTY(EditDefaultsOnly)
	// TSubclassOf<ATile> TileClass;
	TSubclassOf<ABasePawn> BasePawnClass;

	UWorld* World = GetWorld();
	bool flag = false;
	int8 PieceIdx = 0;

	for (int32 x = 0; x < Size; x++)
	{
		for (int32 y = 0; y < Size; y++)
		{
			FVector Location = AGameField::GetRelativeLocationByXYPosition(x, y);
			ATile* TileObj = GetWorld()->SpawnActor<ATile>(TileClass, Location, FRotator::ZeroRotator);
						
			if (TileObj)
			{
				// [A-H][1-8]
				const int32 IdChar = 65 + y;
				const int32 IdNum = x + 1;
				const float TileScale = TileSize / 100;

				TileObj->SetLetterId(FString::Printf(TEXT("%c"), IdChar));
				TileObj->SetNumberId(IdNum);

				UMaterialInterface* Material = ((x + y) % 2) ? MaterialsLight[ETileMaterialType::STANDARD] : MaterialsDark[ETileMaterialType::STANDARD];
				TileObj->GetStaticMeshComponent()->SetMaterial(0, Material);

				// Bottom (Letters)
				if (x == 0 && World)
				{
					AActor* Letter = GetWorld()->SpawnActor<AActor>(LetterNumberClass, Location + FVector(-120, 0, 0), FRotator(0, 90, 0));
					if (Letter)
					{
						Letter->SetActorScale3D(FVector(TileScale * LETTERS_NUMS_SCALE, TileScale * LETTERS_NUMS_SCALE, 1));
						UStaticMeshComponent* LetterMeshComponent = Letter->FindComponentByClass<UStaticMeshComponent>();
						if (LetterMeshComponent)
							LetterMeshComponent->SetMaterial(0, Letters[y]);
					}
				}
				
				// Left (Numbers)
				if (y == 0 && World)
				{
					AActor* Number = GetWorld()->SpawnActor<AActor>(LetterNumberClass, Location + FVector(0, -120, 0), FRotator(0, 90, 0));
					if (Number)
					{
						Number->SetActorScale3D(FVector(TileScale * LETTERS_NUMS_SCALE, TileScale * LETTERS_NUMS_SCALE, 1));
						UStaticMeshComponent* NumberMeshComponent = Number->FindComponentByClass<UStaticMeshComponent>();
						if (NumberMeshComponent)
							NumberMeshComponent->SetMaterial(0, Numbers[x]);
					}
				}

				TileObj->SetActorScale3D(FVector(TileScale, TileScale, TILES_Z));
				TileObj->SetGridPosition(x, y);
				TileArray.Add(TileObj);
				TileMap.Add(FVector2D(x, y), TileObj);

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


/* TODO => rifare commento
 * Function: DistancePieces
 * ----------------------------
 *   Calculate the distance between two pieces given as arguments.
 *	 floor of sqrt((x-x')^2 + (y-y')^2)
 *
 *	 Piece1	const ABasePawn*	1st piece (x,y)
 *	 Piece2 const ABasePawn*	2nd piece (x',y')
 * 
 *	 return		int8	Distance between the two pieces given as arguments
 */
/*bool AGameField::CanReachBlockOpponentKing(const ABasePawn* Piece, const ABasePawn* OpponentKing) const
{
	can reach king = for cardinals => reachable_block = check_direction(king x, y)

	scroll x,y king +-1
	for vertical 
		for horizontal
			tile_to_attack_block(i,j)
			for cardial_directions
				reachable_block = reachable_block | check_direction
				
	return reachable_block
	
} */

/*
 * Function: LoadBoard
 * ----------------------------
 *  Load the board specified as argument
 * 
 *  Board	const TArray<FPieceSaving>&		Board to load
 */
void AGameField::LoadBoard(const TArray<FPieceSaving>& Board)
{
	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		// Graphically restore tiles used to show possible moves 
		for (const auto& move : GameMode->ShownPossibleMoves)
		{
			UMaterialInterface* Material = ((move.first + move.second) % 2) ? MaterialsLight[ETileMaterialType::STANDARD] : MaterialsDark[ETileMaterialType::STANDARD];
			TileArray[move.first * Size + move.second]->GetStaticMeshComponent()->SetMaterial(0, Material);
		}

		// Load chess pieces
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
					PawnArray[i]->SetGridPosition(Board[i].X, Board[i].Y);
					PawnArray[i]->SetStatus(EPawnStatus::ALIVE);
					break;

				case EPawnStatus::DEAD:
					PawnArray[i]->SetActorHiddenInGame(true);
					PawnArray[i]->SetActorEnableCollision(false);
					PawnArray[i]->SetActorTickEnabled(false);
					PawnArray[i]->SetActorLocation(GetRelativeLocationByXYPosition(PawnArray[i]->GetGridPosition()[0], PawnArray[i]->GetGridPosition()[0]) + FVector(0, 0, -20));
					PawnArray[i]->SetGridPosition(-1, -1);
					PawnArray[i]->SetStatus(EPawnStatus::DEAD);
					break;
				}
			}
			else 
			{
				// Chess piece in current PieceArray was not in game when the board was stored
				PawnArray[i]->SetActorHiddenInGame(true);
				PawnArray[i]->SetActorEnableCollision(false);
				PawnArray[i]->SetActorTickEnabled(false);
				PawnArray[i]->SetActorLocation(GetRelativeLocationByXYPosition(PawnArray[i]->GetGridPosition()[0], PawnArray[i]->GetGridPosition()[0]) + FVector(0, 0, -20));
				PawnArray[i]->SetGridPosition(-1, -1);
				PawnArray[i]->SetStatus(EPawnStatus::DEAD);
			}
		}
	}
}



/*
 * Function: IsLineClear
 * ----------------------------
 *  Load the board specified as argument
 * 
 * Line					ELine				Line along the movement is performed (HORIZONTAL | VERTICAL | DIAGONAL)
 * CurrGridPosition		const FVector2D		Current grid position (e.g. [3,2])
 * DeltaX				const int8			Movement delta X 
 * DeltaY				const int8			Movement delta Y
 * 
 * return				bool				true  -> no pieces along the movement
 *											false -> there is a piece along the movement
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


/*
 * Function: IsValidTile
 * ----------------------------
 *  Specify if X and Y are related to a valid tile or not (>= 0 AND < Gameboard.size)
 *	
 *	X	const int8	Coordinate X
 *	Y	const int8	Coordinate Y
 *
 * return	bool	true  -> no pieces along the movement
 *					false -> there is a piece along the movement
 */
bool AGameField::IsValidTile(const int8 X, const int8 Y) const
{
	return X >= 0 && X < Size
		&& Y >= 0 && Y < Size;
}


/* TODO => rifare commento
 * Function: SpawnPawn
 * ----------------------------
 *   Spawn the pawn specified through parameters
 *
 *	 PawnType	EPawnType	: type of the pawn to spawn
 *   PawnColor	EPawnColor	: color of the pawn to spawn
 *   X			int8		: x position of the pawn to spawn
 *	 Y			int8		: y position of the pawn to spawn
 *
 *   return		ABasePawn*	: Pointer to the recently spawned pawn
 */
ABasePawn* AGameField::SpawnPawn(EPawnType PawnType, EPawnColor PawnColor, int8 X, int8 Y, int8 PlayerOwner, bool Simulate)
{
	TSubclassOf<ABasePawn> BasePawnClass;
	ABasePawn* BasePawnObj = nullptr;

	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode && IsValidTile(X, Y))
	{
		ATile* TileObj = GetTileArray()[X * Size + Y];
		TileObj->SetPlayerOwner((PlayerOwner != -1) ? PlayerOwner : GameMode->CurrentPlayer);
		FTileStatus TileStatus = { 
			nullptr, 
			0, 
			{ 0, 0 }, 
			TileObj->GetTileStatus().WhoCanGo,
			EPawnColor::NONE, 
			EPawnType::NONE, 
			ChessEnums::NOT_ASSIGNED 
		};

		TileStatus.PawnType = PawnType;
		TileStatus.PawnColor = PawnColor;
		
		// Calculate spawn location
		FVector Origin; FVector BoxExtent;
		TileObj->GetActorBounds(false, Origin, BoxExtent);
		FVector Location = GetRelativeLocationByXYPosition(X, Y);
		FVector PawnLocation(
			Location.GetComponentForAxis(EAxis::X),
			Location.GetComponentForAxis(EAxis::Y),
			Location.GetComponentForAxis(EAxis::Z) + 2 * BoxExtent.GetComponentForAxis(EAxis::Z) + 0.1
		);

		BasePawnClass = ChessPieces[PawnType];
		BasePawnObj = GetWorld()->SpawnActor<ABasePawn>(BasePawnClass, PawnLocation, FRotator(0, 90, 0));
		if (BasePawnObj)
		{
			if (Simulate)
				BasePawnObj->SetActorHiddenInGame(true);
			BasePawnObj->SetGridPosition(X, Y);
			const float TileScale = TileSize / 100;
			BasePawnObj->SetActorScale3D(FVector(TileScale * CHESS_PIECES_SCALE, TileScale * CHESS_PIECES_SCALE, CHESS_PIECES_Z));
			BasePawnObj->SetPieceNum(PawnArray.Num());
			BasePawnObj->SetType(TileStatus.PawnType);
			BasePawnObj->SetColor(TileStatus.PawnColor);
			BasePawnObj->SetStatus(EPawnStatus::ALIVE);

			TMap<EPawnType, UMaterialInterface*>& ChessPiecesMaterials = PawnColor == EPawnColor::WHITE ? 
				ChessPiecesWhiteMaterials : 
				ChessPiecesBlackMaterials;
			BasePawnObj->GetStaticMeshComponent()->SetMaterial(0, ChessPiecesMaterials[PawnType]);

			PawnArray.Add(BasePawnObj);
			PawnMap.Add(FVector2D(X, Y), BasePawnObj);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ABasePawn Obj is null"));
		}

		TileStatus.Piece = BasePawnObj;
		TileStatus.PlayerOwner = PlayerOwner;
		TileObj->SetPlayerOwner(PlayerOwner);
		TileObj->SetTileStatus(TileStatus);

		if (BasePawnObj->GetType() == EPawnType::KING)
		{
			int8& KingPieceNum = BasePawnObj->GetColor() == EPawnColor::WHITE ?
				GameMode->KingWhitePieceNum :
				GameMode->KingBlackPieceNum;
			KingPieceNum = BasePawnObj->GetPieceNum();
		}
		else if (BasePawnObj->GetType() == EPawnType::ROOK && Y == Size - 1)
		{
			int8& RookRightPieceNum = BasePawnObj->GetColor() == EPawnColor::WHITE ?
				GameMode->RookWhiteRightPieceNum :
				GameMode->RookBlackRightPieceNum;
			RookRightPieceNum = BasePawnObj->GetPieceNum();
		}
	}

	return BasePawnObj;
}


/*
 * Function: DespawnPawn
 * ----------------------------
 *   Despawn pawn which is on the tile specified
 *
 *   X		int8	: x position of the pawn to despawn
 *	 Y		int8	: y position of the pawn to despawn
 */
void AGameField::DespawnPawn(int8 X, int8 Y, bool Simulate)
{
	if (IsValidTile(X, Y))
	{
		ATile* Tile = GetTileArray()[X * Size + Y];
		ABasePawn* Pawn = Tile->GetPawn();
		if (Tile)
		{
			// Reset old tile status
			Tile->SetTileStatus({ 
				nullptr, 
				1, 
				{ 0, 0 }, 
				Tile->GetTileStatus().WhoCanGo, 
				EPawnColor::NONE, 
				EPawnType::NONE
			});
			Tile->SetPlayerOwner(ChessEnums::NOT_ASSIGNED);
		}

		if (Pawn)
		{
			// Update pawn information
			Pawn->SetStatus(EPawnStatus::DEAD);
			Pawn->SetGridPosition(-1, -1);
			if (!Simulate)
			{
				Pawn->SetActorHiddenInGame(true);
				Pawn->SetActorEnableCollision(false);
				Pawn->SetActorTickEnabled(false);
				GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Blue, FString::Printf(TEXT("%f %f pawn has been eaten/despawned"), Pawn->GetGridPosition()[0], Pawn->GetGridPosition()[1]));
			}
		}
	}
}



// Called every frame
/* void AGameField::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

} */