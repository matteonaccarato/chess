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
	TileRestoreMaterialCoordinates = FVector2D(-1, -1);
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


FVector2D AGameField::GetPosition(const FHitResult& Hit) {
	return Cast<ATile>(Hit.GetActor())->GetGridPosition();
}

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



/*
 * Function: GenerateField
 * ----------------------------
 * Generate the game board (graphically and initializing data structures)
 */
void AGameField::GenerateField()
{
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
						SpawnPiece(PawnTypesOnRow[y], EPieceColor::WHITE, x, y, 0);
					else if (x == 1)
						SpawnPiece(EPieceType::PAWN, EPieceColor::WHITE, x, y, 0);
					else if (x == Size - 1)
						SpawnPiece(PawnTypesOnRow[y], EPieceColor::BLACK, x, y, 1);
					else
						SpawnPiece(EPieceType::PAWN, EPieceColor::BLACK, x, y, 1);
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


/*
 * Function: ResetField
 * ----------------------------
 * Reset the game board (graphically and resetting data structures)
 *
 * @param bRestartGame	bool	Flag to notify if starting a new game is required or not after having reset the board
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

		// Clear tile status
		for (ATile* Obj : TileArray)
			DespawnPiece(Obj->GetGridPosition()[0], Obj->GetGridPosition()[1]);
		
		// Load initial board
		TArray<FPieceSaving> InitialBoard;
		int n = PieceArray.Num();
		for (int i = 0; i < n; i++)
		{
			// Initial 32 chess pieces
			if (i < 32)
			{
				int8 x = (PieceArray[i]->GetColor() == EPieceColor::WHITE) ?
					PieceArray[i]->GetPieceNum() / 8
					: PieceArray[i]->GetPieceNum() / 8 + 4;
				int8 y = PieceArray[i]->GetPieceNum() % 8;
				PieceArray[i]->SetGridPosition(x, y);
				PieceArray[i]->SetStatus(EPieceStatus::ALIVE);

				PieceArray[i]->Move(TileArray[x * Size + y], TileArray[x * Size + y]);
				if (PieceArray[i]->GetType() == EPieceType::PAWN)
				{
					PieceArray[i]->SetMaxNumberSteps(2); // restore two steps of first move on pawn
				}

				InitialBoard.Add({
					x,
					y,
					EPieceStatus::ALIVE
				});
			}
			else
			{
				// Here are the pieces added during the game.
				// Destroy actor and reference to it
				if (PieceArray.IsValidIndex(i))
				{
					PieceArray[i]->SelfDestroy();
					PieceArray.RemoveAt(i);
				}
			}
		}
		GameMode->GameSaving.Empty();
		GameMode->CastlingInfoWhite = { false, { false, false } };
		GameMode->CastlingInfoBlack = { false, { false, false } };

		LoadBoard(std::make_tuple(InitialBoard, GameMode->CastlingInfoWhite, GameMode->CastlingInfoBlack));


		// Clear replay
		if (GameMode->ReplayWidget)
		{
			UScrollBox* ScrollBoxWhite = Cast<UScrollBox>(GameMode->ReplayWidget->GetWidgetFromName(TEXT("scr_Replay_white")));
			ScrollBoxWhite->ClearChildren();
			UScrollBox* ScrollBoxBlack = Cast<UScrollBox>(GameMode->ReplayWidget->GetWidgetFromName(TEXT("scr_Replay_black")));
			ScrollBoxBlack->ClearChildren();
		}
	
		GameMode->CheckFlag = EPieceColor::NONE;
		GameMode->MatchStatus = EMatchResult::NONE;
		GameMode->IsGameOver = false;
		GameMode->MoveCounter = 0;
		GameMode->ReplayInProgress = 0;
		if (bRestartGame)
			GameMode->ChoosePlayerAndStartGame();
	}
}


/*
 * Function: LoadBoard
 * ----------------------------
 * Load the board specified as argument
 * 
 * * @param BoardInfo		std::tuple<const TArray<FPieceSaving>&, FCastlingInfo, FCastlingInfo>		Board to load (array of pieces, white castling info, black castling info)
 */
void AGameField::LoadBoard(std::tuple<const TArray<FPieceSaving>&, FCastlingInfo, FCastlingInfo> BoardInfo)
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
		const TArray<FPieceSaving>& Board = std::get<0>(BoardInfo);
		FVector Origin, BoxExtent, Location, PawnLocation;
		for (int8 i = 0; i < PieceArray.Num(); i++)
		{
			if (Board.IsValidIndex(i))
			{
				switch (Board[i].Status)
				{
				case EPieceStatus::ALIVE:
					PieceArray[i]->SetActorHiddenInGame(false);
					PieceArray[i]->SetActorEnableCollision(true);
					PieceArray[i]->SetActorTickEnabled(true);

					TileArray[Board[i].X * Size + Board[i].Y]->GetActorBounds(false, Origin, BoxExtent);
					Location = GetRelativeLocationByXYPosition(Board[i].X, Board[i].Y);
					PawnLocation = FVector(
						Location.GetComponentForAxis(EAxis::X),
						Location.GetComponentForAxis(EAxis::Y),
						Location.GetComponentForAxis(EAxis::Z) + 2 * BoxExtent.GetComponentForAxis(EAxis::Z) + 0.1
					);
					PieceArray[i]->SetActorLocation(PawnLocation);
					PieceArray[i]->SetGridPosition(Board[i].X, Board[i].Y);
					PieceArray[i]->SetStatus(EPieceStatus::ALIVE);
					break;

				case EPieceStatus::DEAD:
					PieceArray[i]->SetActorHiddenInGame(true);
					PieceArray[i]->SetActorEnableCollision(false);
					PieceArray[i]->SetActorTickEnabled(false);
					PieceArray[i]->SetActorLocation(GetRelativeLocationByXYPosition(PieceArray[i]->GetGridPosition()[0], PieceArray[i]->GetGridPosition()[0]) + FVector(0, 0, -20));
					PieceArray[i]->SetGridPosition(-1, -1);
					PieceArray[i]->SetStatus(EPieceStatus::DEAD);
					break;
				}
			}
			else 
			{
				// Chess piece in current PieceArray was not in game when the board was stored
				PieceArray[i]->SetActorHiddenInGame(true);
				PieceArray[i]->SetActorEnableCollision(false);
				PieceArray[i]->SetActorTickEnabled(false);
				PieceArray[i]->SetActorLocation(GetRelativeLocationByXYPosition(PieceArray[i]->GetGridPosition()[0], PieceArray[i]->GetGridPosition()[0]) + FVector(0, 0, -20));
				PieceArray[i]->SetGridPosition(-1, -1);
				PieceArray[i]->SetStatus(EPieceStatus::DEAD);
			}
		}
		GameMode->CastlingInfoWhite = std::get<1>(BoardInfo);
		GameMode->CastlingInfoBlack = std::get<2>(BoardInfo);
	}
}


/*
 * Function: IsValidTile
 * ----------------------------
 * Determine if X and Y are related to a valid tile or not (>= 0 AND < Gameboard.size)
 *	
 * @param X		const int8		Coordinate X
 * @param Y		const int8		Coordinate Y
 *
 * @return		bool			true  -> no pieces along the movement
 *								false -> there is a piece along the movement
 */
bool AGameField::IsValidTile(const int8 X, const int8 Y) const
{
	return X >= 0 && X < Size
		&& Y >= 0 && Y < Size;
}


/*
 * Function: IsLineClear
 * ----------------------------
 * Check if the line from current grid position to a new position (delta_x, delta_y) is clear from other pieces or not
 * 
 * @param Line					ELine				Line along the movement is performed (HORIZONTAL | VERTICAL | DIAGONAL)
 * @param CurrGridPosition		const FVector2D		Current grid position (e.g. [3,2])
 * @param DeltaX				const int8			Movement delta X 
 * @param DeltaY				const int8			Movement delta Y
 * 
 * @return						bool				true  -> no pieces along the movement
 *													false -> there is a piece along the movement
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
 * Function: SpawnPiece
 * ----------------------------
 * Spawn the piece specified through parameters
 *
 * @param PieceType		EPieceType	Type of the piece to spawn
 * @param PieceColor	EPieceColor	Color of the piece to spawn
 * @param X				int8		X position of the piece to spawn
 * @param Y				int8		Y position of the piece to spawn
 * @param PlayerOwner	int8		Player owner of the new piece
 * @param Simulate		bool		Flag if the spawn should be simulated or not (graphically show the piece or not)
 *
 * @return				ABasePiece*	Pointer to the spawned piece
 */
ABasePiece* AGameField::SpawnPiece(EPieceType PieceType, EPieceColor PieceColor, int8 X, int8 Y, int8 PlayerOwner, bool Simulate)
{
	TSubclassOf<ABasePiece> BasePieceClass;
	ABasePiece* BasePieceObj = nullptr;

	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode && IsValidTile(X, Y))
	{
		ATile* TileObj = TileArray[X * Size + Y];
		TileObj->SetPlayerOwner((PlayerOwner != -1) ? PlayerOwner : GameMode->CurrentPlayer);
		FTileStatus TileStatus = { 
			nullptr, 
			0, 
			{ 0, 0 }, 
			TileObj->GetTileStatus().WhoCanGo,
			EPieceColor::NONE, 
			EPieceType::NONE, 
			ChessEnums::NOT_ASSIGNED 
		};

		TileStatus.PieceType = PieceType;
		TileStatus.PieceColor = PieceColor;
		
		// Calculate spawn location
		FVector Origin; FVector BoxExtent;
		TileObj->GetActorBounds(false, Origin, BoxExtent);
		FVector Location = GetRelativeLocationByXYPosition(X, Y);
		FVector PawnLocation(
			Location.GetComponentForAxis(EAxis::X),
			Location.GetComponentForAxis(EAxis::Y),
			Location.GetComponentForAxis(EAxis::Z) + 2 * BoxExtent.GetComponentForAxis(EAxis::Z) + 0.1
		);

		BasePieceClass = ChessPieces[PieceType];
		BasePieceObj = GetWorld()->SpawnActor<ABasePiece>(BasePieceClass, PawnLocation, FRotator(0, 90, 0));
		if (BasePieceObj)
		{
			if (Simulate)
				BasePieceObj->SetActorHiddenInGame(true);
			BasePieceObj->SetGridPosition(X, Y);
			const float TileScale = TileSize / 100;
			BasePieceObj->SetActorScale3D(FVector(TileScale * CHESS_PIECES_SCALE, TileScale * CHESS_PIECES_SCALE, CHESS_PIECES_Z));
			BasePieceObj->SetPieceNum(PieceArray.Num());
			BasePieceObj->SetType(TileStatus.PieceType);
			BasePieceObj->SetColor(TileStatus.PieceColor);
			BasePieceObj->SetStatus(EPieceStatus::ALIVE);

			TMap<EPieceType, UMaterialInterface*>& ChessPiecesMaterials = PieceColor == EPieceColor::WHITE ?
				ChessPiecesWhiteMaterials : 
				ChessPiecesBlackMaterials;
			BasePieceObj->GetStaticMeshComponent()->SetMaterial(0, ChessPiecesMaterials[PieceType]);

			PieceArray.Add(BasePieceObj);
			PieceMap.Add(FVector2D(X, Y), BasePieceObj);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ABasePiece Obj is null"));
		}

		TileStatus.Piece = BasePieceObj;
		TileStatus.PlayerOwner = PlayerOwner;
		TileObj->SetPlayerOwner(PlayerOwner);
		TileObj->SetTileStatus(TileStatus);

		if (BasePieceObj->GetType() == EPieceType::KING)
		{
			int8& KingPieceNum = BasePieceObj->GetColor() == EPieceColor::WHITE ?
				GameMode->KingWhitePieceNum :
				GameMode->KingBlackPieceNum;
			KingPieceNum = BasePieceObj->GetPieceNum();
		}
		else if (BasePieceObj->GetType() == EPieceType::ROOK)
		{
			if (Y == 0)
			{
				int8& RookLeftPieceNum = BasePieceObj->GetColor() == EPieceColor::WHITE ?
					GameMode->RookWhiteLeftPieceNum :
					GameMode->RookBlackLeftPieceNum;
				RookLeftPieceNum = BasePieceObj->GetPieceNum();
			}
			else
			{
				int8& RookRightPieceNum = BasePieceObj->GetColor() == EPieceColor::WHITE ?
					GameMode->RookWhiteRightPieceNum :
					GameMode->RookBlackRightPieceNum;
				RookRightPieceNum = BasePieceObj->GetPieceNum();
			}
		}
	}

	return BasePieceObj;
}


/*
 * Function: DespawnPawn
 * ----------------------------
 * Despawn piece which is on the tile specified
 *
 * @param X			int8	X position of the piece to despawn
 * @param Y			int8	Y position of the piece to despawn
 * @param Simulate	bool	Flag if the spawn should be simulated or not (graphically hide the piece or not)
 */
void AGameField::DespawnPiece(int8 X, int8 Y, bool Simulate)
{
	if (IsValidTile(X, Y))
	{
		ATile* Tile = TileArray[X * Size + Y];
		ABasePiece* Piece = Tile->GetPiece();
		if (Tile)
		{
			// Reset old tile status
			Tile->SetTileStatus({ 
				nullptr, 
				1, 
				{ 0, 0 }, 
				Tile->GetTileStatus().WhoCanGo, 
				EPieceColor::NONE, 
				EPieceType::NONE
			});
			Tile->SetPlayerOwner(ChessEnums::NOT_ASSIGNED);
		}

		if (Piece)
		{
			// Update piece information
			Piece->SetStatus(EPieceStatus::DEAD);
			Piece->SetGridPosition(-1, -1);
			if (!Simulate)
			{
				Piece->SetActorHiddenInGame(true);
				Piece->SetActorEnableCollision(false);
				Piece->SetActorTickEnabled(false);
				// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Blue, FString::Printf(TEXT("%f %f piece has been eaten/despawned"), Piece->GetGridPosition()[0], Piece->GetGridPosition()[1]));
			}
		}
	}
}


/*
 * Function: BackupTiles
 * ----------------------------
 * Do a backup of the tiles status information in the data structured passed as parameter
 *
 * @param TilesStatus  TArray<FTileStatus>&		Ordered collection where to store tiles status information
 */
void AGameField::BackupTiles(TArray<FTileStatus>& TilesStatus) const
{
	for (auto& Tile : TileArray)
	{
		FTileStatus TileStatus = Tile->GetTileStatus();
		TilesStatus.Add(TileStatus);
	}
}


/*
 * Function: RestoreTiles
 * ----------------------------
 * Restore the tiles status information through the data structured passed as parameter
 *
 * @paramn TilesStatusBackup  TArray<FTileStatus>&	Ordered collection where to get tiles status information to restore
 */
void AGameField::RestoreTiles(TArray<FTileStatus>& TilesStatusBackup)
{
	int8 i = 0;
	for (auto& Tile : TileArray)
	{
		if (TilesStatusBackup.IsValidIndex(i))
		{
			Tile->SetTileStatus(TilesStatusBackup[i]);
			Tile->SetPlayerOwner(TilesStatusBackup[i].PlayerOwner);
		}
		i++;
	}
}


/*
 * Function: BackupPiecesInfo
 * ----------------------------
 * Do a backup of pieces information in the data structured passed as parameter
 *
 * @param PiecesInfo  TArray<std::pair<EPieceStatus, FVector2D>>&	Ordered collection (by PieceNum) to store pieces information
 *																	1st element: piece status (ALIVE / DEAD)
 *																	2nd element: grid position
 */
void AGameField::BackupPiecesInfo(TArray<std::pair<EPieceStatus, FVector2D>>& PiecesInfo) const
{
	for (const auto& Piece : PieceArray)
	{
		PiecesInfo.Add(std::make_pair(Piece->GetStatus(), Piece->GetGridPosition()));
	}
}

/*
 * Function: RestorePiecesInfo
 * ----------------------------
 * Restore pieces information in the data structured through the TArray passed as parameter
 *
 * @param PiecesInfoBackup  TArray<std::pair<EPieceStatus, FVector2D>>&		Ordered collection (by PieceNum) to store pieces information
 *																			1st element: piece status (ALIVE / DEAD)
 *																			2nd element: grid position
 */
void AGameField::RestorePiecesInfo(TArray<std::pair<EPieceStatus, FVector2D>>& PiecesInfoBackup)
{
	int8 i = 0;
	int8 PiecesToRemoveCnt = PieceArray.Num() - PiecesInfoBackup.Num();
	for (auto& Piece : PieceArray)
	{
		if (PiecesInfoBackup.IsValidIndex(i))
		{
			Piece->SetStatus(PiecesInfoBackup[i].first);
			Piece->SetGridPosition(PiecesInfoBackup[i].second.X, PiecesInfoBackup[i].second.Y);
		}
		else break;
		i++;
	}

	for (int8 idx = 0; idx < PiecesToRemoveCnt; idx++)
	{
		PieceArray[PieceArray.Num() - 1]->Destroy();
		PieceArray.RemoveAt(PieceArray.Num() - 1);
	}
}



// Called every frame
/* void AGameField::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

} */