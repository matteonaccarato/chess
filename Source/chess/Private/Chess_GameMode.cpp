// Fill out your copyright notice in the Description page of Project Settings.

#include "Chess_GameMode.h"
#include "GameField.h"
#include "Players/Chess_PlayerInterface.h"
#include "Players/Chess_PlayerController.h"
#include "Players/Chess_HumanPlayer.h"
#include "Players/Chess_RandomPlayer.h"
#include "Players/Chess_MiniMaxPlayer.h"
#include "EngineUtils.h"


AChess_GameMode::AChess_GameMode()
{
	PlayerControllerClass = AChess_PlayerController::StaticClass();
	DefaultPawnClass = AChess_HumanPlayer::StaticClass();
	PawnPromotionType = EPawnType::NONE;
	LastGridPosition = FVector2D(-1, -1);
	LastPawnMoveHappened = 0;
	LastCaptureHappened = 0;
	GameInstance = Cast<UChess_GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
}

void AChess_GameMode::BeginPlay()
{
	Super::BeginPlay();

	// Init variables
	IsGameOver = false;
	ReplayInProgress = 0;
	MoveCounter = 0;
	CheckFlag = EPawnColor::NONE;
	MatchStatus = EMatchResult::NONE;

	// Spawn GameField
	if (GameFieldClass && GameInstance)
	{
		GField = GetWorld()->SpawnActor<AGameField>(GameFieldClass);

		// Get and Set Human Player (Camera) Location
		AChess_HumanPlayer* HumanPlayer = Cast<AChess_HumanPlayer>(*TActorIterator<AChess_HumanPlayer>(GetWorld()));
		float CameraPosX = ((GField->TileSize * (GField->Size + ((GField->Size - 1) * GField->NormalizedCellPadding) - (GField->Size - 1))) / 2) - (GField->TileSize / 2);
		FVector CameraPos(CameraPosX, CameraPosX, 1250.0f); // TODO: 1000 da mettere come attributo
		HumanPlayer->SetActorLocationAndRotation(CameraPos, FRotationMatrix::MakeFromX(FVector(0, 0, -1)).Rotator());
	
		// Human player at INDEX 0
		// It is always in the game (it is linked with the camera). 
		// If it does not play, it woks like a spectator
		Players.Add(HumanPlayer); 

		IChess_PlayerInterface* AI = nullptr;
		switch (GameInstance->GetMatchMode())
		{
		case EMatchMode::HUMAN_CPU_RANDOM:
			AI = GetWorld()->SpawnActor<AChess_RandomPlayer>(FVector(), FRotator());
			break;
		case EMatchMode::HUMAN_CPU_MINIMAX:
			AI = GetWorld()->SpawnActor<AChess_MiniMaxPlayer>(FVector(), FRotator());
			break;
		case EMatchMode::RANDOM_RANDOM:
		case EMatchMode::RANDOM_MINIMAX:
		case EMatchMode::MINIMAX_MINIMAX:
			// TODO
			break;
		}
		// TODO AI based on a value passed through parameter (button)
		// Random player
		// auto* AI = GetWorld()->SpawnActor<AChess_RandomPlayer>(FVector(), FRotator());
		// Minimax player

		Players.Add(AI);

		// Create replay widget
		UWorld* World = GetWorld();
		if (World && ReplayWidgetRef)
		{
			ReplayWidget = CreateWidget<UUserWidget>(World, ReplayWidgetRef, FName("Replay"));
			ReplayWidget->AddToViewport(0);
		}

		// Ready to start the game
		this->ChoosePlayerAndStartGame();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameFieldClass OR GameInstance is null"));
	}
}

/*
 * Function: ChoosePlayerAndStartGame
 * ----------------------------
 *   Assigning player numbers and colors and starts the game
 */
void AChess_GameMode::ChoosePlayerAndStartGame()
{
	CurrentPlayer = 0;
	for (int32 i = 0; i < Players.Num(); i++)
	{
		Players[i]->PlayerNumber = i;
		Players[i]->Color = i == CurrentPlayer ? EPawnColor::WHITE : EPawnColor::BLACK;
	}

	CheckFlag = EPawnColor::NONE;
	MatchStatus = EMatchResult::NONE;
	MoveCounter += 1;

	// Operation to init data strcture
	InitTurn();

	Players[FMath::Abs(CurrentPlayer - 1)]->IsMyTurn = false;
	Players[CurrentPlayer]->IsMyTurn = true;
	Players[CurrentPlayer]->OnTurn();
}

/*
 * Function: EndTurn
 * ----------------------------
 *   Handles the end of turn of the player passed as parameter
 *
 *   PlayerNumber		int32	The number of the player whose turn ends
 *   PiecePromotionFlag	bool	Flag to notify piece promotion, required in the computation of the move name
 */
void AChess_GameMode::EndTurn(const int32 PlayerNumber, const bool PiecePromotionFlag)
{
	// -1 to notify end game
	if (PlayerNumber == -1)
	{
		IsGameOver = true;
		// Increment needed to perform replay last two moves properly
		MoveCounter++;
		GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("%d under CHECK MATE"), MatchStatus));

		// End game events
		switch (MatchStatus)
		{
		case EMatchResult::WHITE:
		case EMatchResult::BLACK:
			Players[CurrentPlayer]->OnWin();
			for (int32 i = 0; i < Players.Num(); i++)
				if (i != CurrentPlayer)
					Players[i]->OnLose();
			break;
		default:
			for (int32 i = 0; i < Players.Num(); i++)
				Players[i]->OnDraw();
			break;
		}

		
	
		// Timer to reset the field ( TODO => magari non faccio reset, così l'utente può guardarsi tutto con calma)
		/* FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
			{
				GField->ResetField();
			}, 3, false); */
	}
	else
	{
		if (IsGameOver || PlayerNumber != CurrentPlayer)
			return;

		Players[PlayerNumber]->IsMyTurn = false;

		// Clean opponent's attackable tiles, they will be overwritten the next turn,
		// so the previous state is useless
		for (auto& InnerTArray : Players[FMath::Abs(CurrentPlayer - 1)]->AttackableTiles)
		{
			InnerTArray.Empty();
		} 
		Players[FMath::Abs(CurrentPlayer - 1)]->AttackableTiles.Empty();

		
		// Clear AttackableFrom and WhoCanGo attribute of each tile
		for (ATile* Tile : GField->GetTileArray())
		{
			FTileStatus TileStatus = Tile->GetTileStatus();
			TileStatus.AttackableFrom[FMath::Abs(CurrentPlayer - 1)] = 0;
			TileStatus.WhoCanGo.Empty();
			Tile->SetTileStatus(TileStatus);
		}


		// Save position (X,Y) of each piece in order to store current board situation
		//	the index in the BoardSaving TArray identifies each piece
		//	e.g. White rook at bottom-left is at index 0,
		//	     White knight next to it is at 1,
		//		 ...
		//  e.g. [{0,0}, {2,0}, {1,3}, ...] means the Bottom-left white rook is at (0,0), the knight next to it is at (2,0), ...
		TArray<FPieceSaving> BoardSaving;
		for (auto& Piece : GField->PawnArray)
		{
			BoardSaving.Add({
				static_cast<int8>(Piece->GetGridPosition()[0]),
				static_cast<int8>(Piece->GetGridPosition()[1]),
				Piece->GetStatus()
			});
		}

		// Save current board and add it to the game history
		CurrentBoard = BoardSaving;
		GameSaving.Add(BoardSaving);



		// Operation to init data strcture
		InitTurn();


		if (CheckFlag != EPawnColor::NONE)
			GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Yellow, FString::Printf(TEXT("King under check | %d"), CheckFlag));
		

		// Match situation
		MatchStatus = ComputeMatchResult(WhitePiecesCanMove, BlackPiecesCanMove);

		if (LastPiece->GetType() == EPawnType::PAWN)
			LastPawnMoveHappened = MoveCounter;
		if (LastEatFlag)
			LastCaptureHappened = MoveCounter;

		ReplayManager::AddToReplay(this, LastPiece, LastEatFlag, PiecePromotionFlag);


		if (MatchStatus != EMatchResult::NONE)
			EndTurn(-1);
		else
			TurnNextPlayer();
	}
}

EMatchResult AChess_GameMode::ComputeMatchResult(TArray<int8>& WhitePieces, TArray<int8>& BlackPieces)
{
	EMatchResult Result = EMatchResult::NONE;

	ABasePawn* WhiteKing = GField->PawnArray[KingWhitePieceNum];
	ABasePawn* BlackKing = GField->PawnArray[KingBlackPieceNum];
	if (WhitePieces.Num() == 0)
		if (GField->TileArray[WhiteKing->GetGridPosition()[0] * GField->Size + WhiteKing->GetGridPosition()[1]]->GetTileStatus().AttackableFrom)
			Result = EMatchResult::WHITE;
		else
			Result = EMatchResult::STALEMATE;
	else if (BlackPieces.Num() == 0)
		if (GField->TileArray[BlackKing->GetGridPosition()[0] * GField->Size + BlackKing->GetGridPosition()[1]]->GetTileStatus().AttackableFrom)
			Result = EMatchResult::BLACK;
		else
			Result = EMatchResult::STALEMATE;
	else if (SameConfigurationBoard(5))
		Result = EMatchResult::FIVEFOLD_REPETITION;
	else if (SeventyFive_MoveRule())
		Result = EMatchResult::SEVENTY_FIVE_MOVE_RULE;
	else if (ImpossibilityToCheckmate())
		Result = EMatchResult::INSUFFICIENT_MATERIAL;

	return Result;
}

/*
 * Function: GetNextPlayer
 * ----------------------------
 *   Returns next player index
 *
 *   Player		int32	Number of the current player
 * 
 *   return: next player index
 */
int32 AChess_GameMode::GetNextPlayer(int32 Player)
{
	// TODO => sostituire con FMath::Abs(Player - 1)
	Player++;
	if (!Players.IsValidIndex(Player))
	{
		Player = 0;
	}
	return Player;
}

/*
 * Function: TurnNextPlayer
 * ----------------------------
 *   Starts the next player turn
 */
void AChess_GameMode::TurnNextPlayer()
{
	MoveCounter += 1;
	CurrentPlayer = GetNextPlayer(CurrentPlayer);

	Players[CurrentPlayer]->IsMyTurn = true;
	Players[CurrentPlayer]->OnTurn();
}

void AChess_GameMode::InitTurn()
{
	// Clear possible moves
	for (auto& InnerArray : TurnPossibleMoves)
		InnerArray.Empty();
	TurnPossibleMoves.Empty();

	// Clear tile status (AttackableFrom and WhoCanGo variables)
	for (auto& Tile : GField->TileArray)
	{
		FTileStatus TileStatus = Tile->GetTileStatus();
		TileStatus.AttackableFrom[0] = 0; TileStatus.AttackableFrom[1] = 0;
		TileStatus.WhoCanGo.Empty();
		Tile->SetTileStatus(TileStatus);
	}

	WhitePiecesCanMove.Empty();
	BlackPiecesCanMove.Empty();

	// TODO => compute possible moves of current player 
	// (tarray di tarray per mosse possibli, accedo con pieceNum
	// bool BlackCanMove = false;
	// bool WhiteCanMove = false;
	for (const auto& Piece : GField->PawnArray)
	{
		//EPawnColor PreviousCheckFlag = CheckFlag;
		TArray<std::pair<int8, int8>> Tmp = ShowPossibleMoves(Piece, false, true, true);
		TurnPossibleMoves.Add(Tmp);
		//CheckFlag = PreviousCheckFlag;
		if (Tmp.Num() > 0)
		{
			switch (Piece->GetColor())
			{
			case EPawnColor::WHITE: WhitePiecesCanMove.Add(Piece->GetPieceNum()); break;
			case EPawnColor::BLACK: BlackPiecesCanMove.Add(Piece->GetPieceNum()); break;
			}
		}
	}

	CheckFlag = CheckKingsUnderAttack();
}

/*
 * Function: SetPawnPromotionChoice
 * ----------------------------
 *   Promote the pawn with a new chess piece (type passed as parameter)
 *
 *   PawnType	EPawnType	: type of the new chess piece to spawn
 */
void AChess_GameMode::SetPawnPromotionChoice(EPawnType PawnType)
{	
	int8 X = LastGridPosition.X;
	int8 Y = LastGridPosition.Y;
	PawnPromotionType = PawnType;

	// Despawn & Spawn pawn
	GField->DespawnPawn(X, Y);
	ABasePawn* PawnTemp = GField->SpawnPawn(PawnType, Players[CurrentPlayer]->Color, X, Y);
	
	if (PawnPromotionWidget != nullptr)
		PawnPromotionWidget->RemoveFromParent();

	// TODO => vedere se si può alleggerire (posso rimuovere, viene fatto quando l'ai inizia il turno)
	IsCheck();
	// AddToReplay(PawnTemp, LastEatFlag, true);
	LastPiece = PawnTemp;
	LastEatFlag = LastEatFlag;
	EndTurn(CurrentPlayer, true);
}

/*
 * Function: ShowPossibleMoves
 * ----------------------------
 *   Calculates the eligible moves of the chess piece passed as parameter
 *
 *   Pawn					ABasePawn*		: pawn on which to calculate the eligible moves
 *	 ShowAttackable			bool = false	: flag to determine if only the attackable tiles should be taken into account 
 *												(possible tiles to move on and attackable tiles are different for pawns)
 *	 CheckCheckFlag			bool = false	: flag to determine if checking the new check situation should be evaluated
 *	 UpdateWhoCanGoFlag		bool = false	: flag to notify if updating WhoCanGo: FTileStatus, 
 *												attribute of the tile taken into account should be updated with the current piece
 *
 *   return: TArray made of new possible X,Y of the chess piece
 */
TArray<std::pair<int8, int8>> AChess_GameMode::ShowPossibleMoves(ABasePawn* Pawn, const bool ShowAttackable, const bool CheckCheckFlag, const bool UpdateWhoCanGoFlag)
{
	TArray<std::pair<int8, int8>> PossibleMoves;
	if (Pawn && Pawn->GetStatus() == EPawnStatus::ALIVE)
	{
		FVector2D CurrPawnGridPosition = Pawn->GetGridPosition();
		const int8 X = CurrPawnGridPosition[0];
		const int8 Y = CurrPawnGridPosition[1];

		TArray<ECardinalDirection> PawnDirections = Pawn->GetCardinalDirections();
		int8 MaxSteps = Pawn->GetMaxNumberSteps();
		int8 XOffset = 0, YOffset = 0;

		// TODO => è senza controllo della direzione
		// Do the true flow if: 
		//	- showing the attackable tiles is not required (it means computing all possible moves)
		//	-  OR the opponent king is reachable (distance between the piece and the opponent king is under the max number of steps the piece can make)
		//  -  OR the piece is a knight (the rule above doesn't work for it, it has 1 as max number of steps)
		// TODO => valutare di mettere max number of steps del cavallo come quella reale per raggiungere il re (posso togliere la terza condizione) 
		if (!ShowAttackable
			|| (GField->DistancePieces(Pawn, GField->PawnArray[Pawn->GetColor() == EPawnColor::BLACK ? 4 : 28]) <= Pawn->GetMaxNumberSteps())
			|| Pawn->GetType() == EPawnType::KNIGHT)
		{
			for (const auto& PawnDirection : PawnDirections)
			{
				// TODO => fare stesso check di prima controllando la direzione  
				for (int8 i = 1; i <= MaxSteps; i++)
				{
					std::pair<int8, int8> Offsets = GField->GetXYOffset(i, PawnDirection, Pawn->GetColor());
					XOffset = Offsets.first;
					YOffset = Offsets.second;

					// Evaluate if this move is valid or not
					if (IsValidMove(Pawn, X + XOffset, Y + YOffset, true, ShowAttackable, CheckCheckFlag))
					{
						// Add the VALID move to result TArray
						PossibleMoves.Add(std::make_pair(X + XOffset, Y + YOffset));
						FTileStatus TileStatus = GField->GetTileArray()[(X + XOffset) * GField->Size + Y + YOffset]->GetTileStatus();

						// Check needed to avoid pawns eat on straight line
						if (!(Pawn->GetType() == EPawnType::PAWN && PawnDirection == ECardinalDirection::NORTH))
						{
							// Index 0 means attackable from whites
							// Index 1 means attackable from blacks 
							TileStatus.AttackableFrom[(static_cast<int>(Pawn->GetColor()) == 1) ? 0 : 1] += 1;
						}
						if (UpdateWhoCanGoFlag)
							TileStatus.WhoCanGo.Add(Pawn); // TODO => NOT WORKING, used to compute correct move name

						GField->GetTileArray()[(X + XOffset) * GField->Size + Y + YOffset]->SetTileStatus(TileStatus); // TODO => player owner as ENUM
					}
					else {
						// If the piece cannot do a move with n steps in a direction (due to the presence of another piece),
						//  it will not do it with n+1 steps in that direction
						if (GField->IsValidTile(X + XOffset, Y + YOffset) && !GField->GetTileArray()[(X + XOffset) * GField->Size + Y + YOffset]->GetTileStatus().EmptyFlag)
							break;
					}
				}
			}

			// Castling handling
			if (Pawn->GetType() == EPawnType::KING)
			{
				// short castling and long castling
				for (ECardinalDirection Direction : { ECardinalDirection::EAST, ECardinalDirection::WEST })
				{
					std::pair<int8, int8> Offsets = GField->GetXYOffset(2, Direction, Pawn->GetColor());
					XOffset = Offsets.first;
					YOffset = Offsets.second;
					// Evaluate if this move is valid or not
					if (IsValidMove(Pawn, X + XOffset, Y + YOffset, true, ShowAttackable, CheckCheckFlag, true))
					{
						// Add the VALID move to result TArray
						PossibleMoves.Add(std::make_pair(X + XOffset, Y + YOffset));
					}
				}
			}
		}
	}

	return PossibleMoves;
}


/*
 * Function: IsCheck
 * ----------------------------
 *   Computes (if no parameters are passed) and determines if a king is under king
 * 
 *	TODO => descrivere algoritmo
 *
 *	 Pawn	ABasePawn* = nullptr	: pawn to move on new x and new y and compute check situation
 *   NewX	const int8 = -1			: eventually new x position of the pawn
 *	 NewY	const int8 = -1			: eventually new y position of the pawn
 *
 *   return: Pointer to the recently spawned pawn
 */
EPawnColor AChess_GameMode::IsCheck(ABasePawn* Pawn, const int8 NewX, const int8 NewY)
{
	EPawnColor ColorAttacker = CurrentPlayer ? EPawnColor::BLACK : EPawnColor::WHITE;	

	// If no parameters are passed, calculate check situation by computing attackable tiles and evaluating MatchStatus
	if (Pawn == nullptr)
	{
		ComputeAttackableTiles();
		
		if (MatchStatus == EMatchResult::NONE)
			CheckFlag = CheckKingsUnderAttack();
		
		return CheckFlag;
	}
	else
	{
		// Evaluating the new situation by simulating the move of Piece on [NewX, NewY]
		if (GField->IsValidTile(NewX, NewY)
			&& IsValidMove(Pawn, NewX, NewY, true, false, false))
		{
			// Backup old tile and new tile
			FVector2D Backup_OldPosition = Pawn->GetGridPosition();
			ATile* OldTile = GField->GetTileArray()[Backup_OldPosition[0] * GField->Size + Backup_OldPosition[1]];
			ATile* NewTile = GField->GetTileArray()[NewX * GField->Size + NewY];

			ABasePawn* Backup_OldPawn = OldTile->GetPawn();
			int32 Backup_OldPlayerOwner = OldTile->GetPlayerOwner();
			FTileStatus Backup_OldTileStatus = OldTile->GetTileStatus();

			int32 Backup_NewPlayerOwner = NewTile->GetPlayerOwner();
			FTileStatus Backup_NewTileStatus = NewTile->GetTileStatus();

			ABasePawn* PawnToEat = NewTile->GetPawn();
			EPawnStatus Backup_PawnToEatStatus;
			FVector2D Backup_PawnToEatGridPosition;
			if (PawnToEat)
			{
				Backup_PawnToEatStatus = PawnToEat->GetStatus();
				Backup_PawnToEatGridPosition = PawnToEat->GetGridPosition();
				PawnToEat->SetStatus(EPawnStatus::DEAD);
				PawnToEat->SetGridPosition(-1, -1);
			}

			// Backup tile status
			TArray<std::pair<std::pair<int8, int8>, FTileStatus>> TileStatusBackup;
			for (ATile* Tile : GField->GetTileArray())
			{
				int8 X = Tile->GetGridPosition()[0];
				int8 Y = Tile->GetGridPosition()[1];
				FTileStatus TileStatus = Tile->GetTileStatus();
				TileStatusBackup.Add(std::make_pair(std::make_pair(X, Y), TileStatus));

				TileStatus.AttackableFrom[0] = 0; TileStatus.AttackableFrom[1] = 0;
				Tile->SetTileStatus(TileStatus);
			}

			// Simulate move
			Pawn->Move(OldTile, NewTile, true);

			// Compute attackable tile with the new move
			ComputeAttackableTiles(); 

			EPawnColor OldCheckFlag = CheckFlag;
			EPawnColor NewCheckFlag = CheckKingsUnderAttack();

			// Undo and restore backed-up info
			for (const auto& pair : TileStatusBackup)
				GField->GetTileArray()[pair.first.first * GField->Size + pair.first.second]->SetTileStatus(pair.second);
			
			OldTile->SetPlayerOwner(Backup_OldPlayerOwner);
			OldTile->SetTileStatus(Backup_OldTileStatus);

			NewTile->SetPlayerOwner(Backup_NewPlayerOwner);
			NewTile->SetTileStatus(Backup_NewTileStatus);
			Pawn->SetGridPosition(Backup_OldPosition[0], Backup_OldPosition[1]);

			if (PawnToEat)
			{
				PawnToEat->SetStatus(Backup_PawnToEatStatus);
				PawnToEat->SetGridPosition(Backup_PawnToEatGridPosition[0], Backup_PawnToEatGridPosition[1]);
			}

			CheckFlag = OldCheckFlag;

			return NewCheckFlag;
		}
		return CheckFlag;
	}
}

/*
 * Function: ComputeAttackableTiles
 * ----------------------------
 *   Computes all attackable tiles based on the current situation of data structure 
 *		( GameMode->GField->TileArray, GameMode->GField->PawnArray )
 *	 updates the AttackableTileStatus attribute of each Tile where necessary
 */
void AChess_GameMode::ComputeAttackableTiles()
{
	ComputeAllPossibleMoves(EPawnColor::BOTH, true, false);
}

// 1st TODO => inglobarla con computeattackabletiles (una richiama l'altra con flag di attackable)
// 2nd TODO => aggiungere reset dell'attackable status delle tile (toglierlo dal punto in cui chiamo questa funzione 
TArray<std::pair<int8, TArray<std::pair<int8, int8>>>> AChess_GameMode::ComputeAllPossibleMoves(EPawnColor Color, const bool ShowAttackable, const bool CheckCheck, const bool UpdateTurnMoves)
{
	// < piece_num , < newx, newy > >
	TArray<std::pair<int8, TArray<std::pair<int8, int8>>>> PiecesMoves;
	
	// Reset AttackableFrom info
	for (auto& Tile : GField->TileArray)
	{
		FTileStatus TileStatus = Tile->GetTileStatus();
		TileStatus.AttackableFrom[0] = 0; TileStatus.AttackableFrom[1] = 0;
		Tile->SetTileStatus(TileStatus);
	} 

	for (auto& Piece : GField->PawnArray)
	{
		if (Piece->GetStatus() == EPawnStatus::ALIVE && (Piece->GetColor() == Color || Color == EPawnColor::BOTH))
		{
			TArray<std::pair<int8, int8>> Tmp = ShowPossibleMoves(Piece, ShowAttackable, CheckCheck);
			if (!ShowAttackable && UpdateTurnMoves)
				TurnPossibleMoves.Add(Tmp);
			if (Tmp.Num() > 0)
				PiecesMoves.Add(std::make_pair(Piece->GetPieceNum(), Tmp));
		}
	}
	return PiecesMoves;
}

/*
 * Function: CheckKingUnderAttack
 * ----------------------------
 *   Looks if each king is on a tile which is attackable from an opponent piece
 *
 *   return: EPawnColor		Color of the king under check (NONE, WHITE, BLACK, BOTH). BOTH means a NOT valid situation
 */
EPawnColor AChess_GameMode::CheckKingsUnderAttack() const
{
	EPawnColor TmpCheckFlag = EPawnColor::NONE;
	if (GField->PawnArray.IsValidIndex(KingWhitePieceNum)
		&& GField->PawnArray.IsValidIndex(KingBlackPieceNum))
	{
		for (const auto& King : { 
			GField->PawnArray[KingWhitePieceNum], 
			GField->PawnArray[KingBlackPieceNum] })
		{
			FVector2D KingXY = King->GetGridPosition();
			int8 OpponentIdx = (static_cast<int>(King->GetColor()) == 1) ? 1 : 0;
			if (King->GetStatus() == EPawnStatus::ALIVE && GField->IsValidTile(KingXY[0], KingXY[1]))
			{
				if (GField->GetTileArray()[KingXY[0] * GField->Size + KingXY[1]]->GetTileStatus().AttackableFrom[OpponentIdx])
				{
					if (TmpCheckFlag == EPawnColor::NONE)
						TmpCheckFlag = King->GetColor();
					else
						TmpCheckFlag = EPawnColor::BOTH; // Error kings cannot be under check at the same time
				}
			}
		}
	}

	return TmpCheckFlag;
}

/*
 * Function: MakeMove
 * ----------------------------
 *   It makes the move specified through paramters
 * 
 *	 Piece		ABasePawn*		Piece to move
 *	 NewX		const int8		X to end to
 *	 NewY		const int8		Y to end to
 *	 Simulate	bool = false	Determines if the move is just a simulation or not.
 *								If so, graphically moving the piece is not required
 *
 *   return		bool	Flag to notify if a capture happened	
 */
bool AChess_GameMode::MakeMove(ABasePawn* Piece, const int8 NewX, const int8 NewY, bool Simulate)
{
	bool EatFlag = false;
	int8 OldX = Piece->GetGridPosition()[0];
	int8 OldY = Piece->GetGridPosition()[1];
	if (GField->IsValidTile(OldX, OldY)
		&& GField->IsValidTile(NewX, NewY))
	{
		TArray<ATile*> TilesArray = GField->GetTileArray();

		// EatFlag is true if the Tile->PawnColor is the opposite of the black pawn
		// e.g. Tile->PawnwColor = 1 (white) , Pawn->Color = -1 => EatFlag = true
		// e.g. Tile->PawnwColor = 0 (empty) , Pawn->Color = -1 => EatFlag = flag
		EatFlag = static_cast<int>(TilesArray[NewX * GField->Size + NewY]->GetTileStatus().PawnColor) == -static_cast<int>(Piece->GetColor());
		if (EatFlag)
		{
			ABasePawn* PawnToEat = TilesArray[NewX * GField->Size + NewY]->GetPawn();
			if (PawnToEat && PawnToEat->GetType() != EPawnType::KING)
				GField->DespawnPawn(PawnToEat->GetGridPosition()[0], PawnToEat->GetGridPosition()[1], Simulate);
			if (PawnToEat && PawnToEat->GetType() == EPawnType::KING)
				EatFlag = false;

			if (PawnToEat && PawnToEat->GetType() == EPawnType::ROOK)
			{
				FCastlingInfo& CastlingInfo = PawnToEat->GetColor() == EPawnColor::WHITE ? CastlingInfoWhite : CastlingInfoBlack;
				CastlingInfo.RooksMoved[PawnToEat->GetPieceNum() % 7 == 0 ? 1 : 0] = true;
			}
		}

		// Clear starting tile (no player owner, no pawn on it, ...)
		// Update ending tile (new player owner, new tile status, new pawn)
		Piece->Move(TilesArray[OldX * GField->Size + OldY], TilesArray[NewX * GField->Size + NewY], Simulate);


		// Castling Handling (King moves by two tiles)
		FCastlingInfo& CastlingInfo = Piece->GetColor() == EPawnColor::WHITE ? CastlingInfoWhite : CastlingInfoBlack;
		if (Piece->GetType() == EPawnType::KING
			&& FMath::Abs(NewY - OldY) == 2)
		{
			// Move the rook
			bool ShortCastling = (NewY - OldY) > 0;
			int8 RookX = Piece->GetColor() == EPawnColor::WHITE ? 0 : 7;
			int8 OldRookY = ShortCastling ? 7 : 0;
			ATile* OldRookTile = GField->TileArray[RookX * GField->Size + OldRookY];
			ABasePawn* RookToMove = OldRookTile->GetPawn();

			int8 NewRookY = OldRookY + (ShortCastling ? -2 : 3);
			if (GField->IsValidTile(RookX, NewRookY))
			{
				ATile* NewRookTile = GField->TileArray[RookX * GField->Size + NewRookY];
				if (RookToMove)
				{
					RookToMove->Move(OldRookTile, NewRookTile, Simulate);
					CastlingInfo.RooksMoved[NewRookY == 0 ? 0 : 1] = true;
				}
			}
		}
		
		switch (Piece->GetType())
		{
		case EPawnType::PAWN: Piece->SetMaxNumberSteps(1); break;
		case EPawnType::KING: CastlingInfo.KingMoved = true; break;
		case EPawnType::ROOK: CastlingInfo.RooksMoved[OldY == 0 ? 0 : 1] = true; break;
		}

		// Update last move (useful when doing pawn promotion)
		LastPiece = Piece;
		LastGridPosition = FVector2D(NewX, NewY);
		PreviousGridPosition = FVector2D(OldX, OldY);
		LastEatFlag = EatFlag;
	}
	return EatFlag;
}


// TODO make it const
/*
 * Function: IsValidMove
 * ----------------------------
 *   Computes if a move (specified through parameters) is valid or not
 * 
 *	TODO => descrivere algoritmo
 *
 *	 Pawn				ABasePawn*			: pawn to move on new x and new y and compute check situation
 *   NewX				const int8			: new x position of the piece
 *	 NewY				const int8			: new y position of the piece
 *	 ShowAttackable		bool = false		: flag to determine if only the attackable tiles should be taken into account 
												(possible tiles to move on and attackable tiles are different for pawns)
 *	 CheckCheckFlag		const bool = true	: flag to determine if checking check state after this move is required
 *   CastlingFlag		const bool = false	: flag to determine if checking castling situation is required
 * 
 *   return 			bool				: Determines if a move is valid or not
 */
bool AChess_GameMode::IsValidMove(ABasePawn* Pawn, const int8 NewX, const int8 NewY, const bool TestFlag, const bool ShowAttackable, const bool CheckCheckFlag, const bool CastlingFlag)
{
	bool IsValid = false;

	if (Pawn 
		&& GField->IsValidTile(NewX, NewY)
		&& !(NewX == Pawn->GetGridPosition()[0] && NewY == Pawn->GetGridPosition()[1]))
	{
		ATile* NewTile = GField->TileArray[NewX * GField->Size + NewY];

		// If the colors of the pieces on the tile are -1 and 1 or viceversa, 
		//  it means that a piece is eating another one
		bool EatFlag = static_cast<int>(NewTile->GetTileStatus().PawnColor) == -static_cast<int>(Pawn->GetColor());
	
		FVector2D NewGridPosition = NewTile->GetGridPosition();
		FVector2D CurrGridPosition = Pawn->GetGridPosition();
	
		EPawnColor DirectionFlag = Pawn->GetColor();
		int8 DeltaX = (NewGridPosition[0] - CurrGridPosition[0]); 
		int8 DeltaY = NewGridPosition[1] - CurrGridPosition[1];

		// Compute the eligibily of the move based on piece type and starting/ending grid position
		if ((NewTile->GetTileStatus().EmptyFlag && !EatFlag) 
			|| (EatFlag && !NewTile->GetTileStatus().EmptyFlag && (NewTile->GetTileStatus().PawnColor != Pawn->GetColor())))
		{
			// Perform checks based on directions which the piece can perform
			switch (Pawn->GetType())
			{
			case EPawnType::PAWN:
				if (EatFlag || ShowAttackable)
					IsValid = GField->CheckDirection(EDirection::DIAGONAL, Pawn, NewGridPosition, CurrGridPosition, TestFlag);
				else
					IsValid = GField->CheckDirection(EDirection::FORWARD, Pawn, NewGridPosition, CurrGridPosition, TestFlag);
				break;

			case EPawnType::ROOK:
				IsValid = GField->CheckDirection(EDirection::FORWARD, Pawn, NewGridPosition, CurrGridPosition);
				IsValid = IsValid || GField->CheckDirection(EDirection::BACKWARD, Pawn, NewGridPosition, CurrGridPosition);
				IsValid = IsValid || GField->CheckDirection(EDirection::HORIZONTAL, Pawn, NewGridPosition, CurrGridPosition);
				break;

			case EPawnType::KNIGHT:
				IsValid = GField->CheckDirection(EDirection::KNIGHT, Pawn, NewGridPosition, CurrGridPosition);
				break;

			case EPawnType::BISHOP:
				IsValid = GField->CheckDirection(EDirection::DIAGONAL, Pawn, NewGridPosition, CurrGridPosition);
				break;

			case EPawnType::QUEEN: // so it works like an OR for Queen and King
			case EPawnType::KING:
				int8 AttackableFrom[2] = {
					GField->GetTileArray()[NewGridPosition[0] * GField->Size + NewGridPosition[1]]->GetTileStatus().AttackableFrom[0],
					GField->GetTileArray()[NewGridPosition[0] * GField->Size + NewGridPosition[1]]->GetTileStatus().AttackableFrom[1] 
				};
				
				if (CastlingFlag)
				{
					FCastlingInfo CastlingInfo = Pawn->GetColor() == EPawnColor::WHITE ? CastlingInfoWhite : CastlingInfoBlack;
					
					// If delta y > 0 => then look for the rook on the right side of the king, otherwise the left one 
					int8 RookIdx = DeltaY > 0 ? 1 : 0; 
					
					// 1st validation: if the line is clear and neither king and rook have already moved
					// If DeltaY < 0, it means long castling should be handled, so it is necessary to check that all the tiles between left rook and king are empty
					//	e.g. Short Castling:	CheckLineClearDeltaY = DeltaY + 1	= +3		(From the tile which the king is on to the two at its right)
					//		 Long  Castling:	CheckLineClearDeltaY = DeltaY - 2	= -4	(From the tile which the king is on to the three at its left)
					IsValid = !CastlingInfo.KingMoved
						&& !CastlingInfo.RooksMoved[RookIdx]
						&& GField->IsLineClear(ELine::HORIZONTAL, CurrGridPosition, DeltaX, DeltaY + (DeltaY > 0 ? 1 : -2));
					
					// 2nd validation: looking for every tile involved in the movement if it is attackable from opponent piece,
					//	if so, castling is not allowed (king would passed from, through, to a check situation)
					for (int8 i = 0; i <= FMath::Abs(DeltaY) && IsValid; i++)
					{
						if (GField->IsValidTile(NewGridPosition[0], CurrGridPosition[1] + i * FMath::Sign(DeltaY)))
						{	
							EPawnColor PreviousCheckFlag = CheckFlag;
							EPawnColor NewCheckFlag = IsCheck(
								Pawn, 
								NewGridPosition[0], 
								CurrGridPosition[1] + i * FMath::Sign(DeltaY)
							);
							CheckFlag = PreviousCheckFlag;

							// Possible valid situations after having checked the new check situation
							IsValid = NewCheckFlag == EPawnColor::NONE
								|| (NewCheckFlag == EPawnColor::BLACK && Pawn->GetColor() != EPawnColor::BLACK)
								|| (NewCheckFlag == EPawnColor::WHITE && Pawn->GetColor() != EPawnColor::WHITE);
						}
					}
				}
				else if (!(Pawn->GetType() == EPawnType::KING && AttackableFrom[(static_cast<int>(Pawn->GetColor()) == 1) ? 1 : 0]))
				{
					IsValid = GField->CheckDirection(EDirection::FORWARD, Pawn, NewGridPosition, CurrGridPosition);
					IsValid = IsValid || GField->CheckDirection(EDirection::BACKWARD, Pawn, NewGridPosition, CurrGridPosition);
					IsValid = IsValid || GField->CheckDirection(EDirection::HORIZONTAL, Pawn, NewGridPosition, CurrGridPosition);
					IsValid = IsValid || GField->CheckDirection(EDirection::DIAGONAL, Pawn, NewGridPosition, CurrGridPosition);
				}
				break;
			}
		}

		// If the move is valid and checking the new check state is required,
		//	computing the new check situation
		if (IsValid && CheckCheckFlag)
		{
			// Check state after having moved the Piece in the new grid position
			if (GField->IsValidTile(NewGridPosition[0], NewGridPosition[1]) && Pawn->GetStatus() == EPawnStatus::ALIVE)
			{
				EPawnColor PreviousCheckFlag = CheckFlag;
				EPawnColor NewCheckFlag = IsCheck(Pawn, NewGridPosition[0], NewGridPosition[1]);
				CheckFlag = PreviousCheckFlag;

				// Possible valid situations after having checked the new check situation
				IsValid = NewCheckFlag == EPawnColor::NONE
					|| (NewCheckFlag == EPawnColor::BLACK && Pawn->GetColor() != EPawnColor::BLACK)
					|| (NewCheckFlag == EPawnColor::WHITE && Pawn->GetColor() != EPawnColor::WHITE); 
			}
		}
	}

	return IsValid;
}





/*
 * Function: BackupTiles
 * ----------------------------
 * It does a backup of the tiles status information in the data structured passed as parameter
 *
 * TilesStatus  TArray<FTileStatus>&	Ordered collection to store tiles status information
 */
void AChess_GameMode::BackupTiles(TArray<FTileStatus>& TilesStatus) const
{
	for (auto& Tile : GField->TileArray)
	{
		FTileStatus TileStatus = Tile->GetTileStatus();
		TilesStatus.Add(TileStatus);
	}
}

/*
 * Function: RestoreTiles
 * ----------------------------
 * It restores the tiles status information through the data structured passed as parameter
 *
 * TilesStatusBackup  TArray<FTileStatus>&	Ordered collection to store tiles status information
 */
void AChess_GameMode::RestoreTiles(TArray<FTileStatus>& TilesStatusBackup)
{
	int8 i = 0;
	for (auto& Tile : GField->TileArray)
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
 * It does a backup of pieces information in the data structured passed as parameter
 *
 * PiecesInfo  TArray<std::pair<EPawnStatus, FVector2D>>&	Ordered collection (by PieceNum) to store pieces information
 *															1st element: piece status (ALIVE / DEAD)
 *															2nd element: grid position
 */
void AChess_GameMode::BackupPiecesInfo(TArray<std::pair<EPawnStatus, FVector2D>>& PiecesInfo) const
{
	for (const auto& Piece : GField->PawnArray)
	{
		PiecesInfo.Add(std::make_pair(Piece->GetStatus(), Piece->GetGridPosition()));
	}
}

/*
 * Function: RestorePiecesInfo
 * ----------------------------
 * It restores pieces information in the data structured through the TArray passed as parameter
 *
 * PiecesInfoBackup  TArray<std::pair<EPawnStatus, FVector2D>>&		Ordered collection (by PieceNum) to store pieces information
 *																	1st element: piece status (ALIVE / DEAD)
 *																	2nd element: grid position			
 */
void AChess_GameMode::RestorePiecesInfo(TArray<std::pair<EPawnStatus, FVector2D>>& PiecesInfoBackup)
{
	int8 i = 0;
	for (auto& Piece : GField->PawnArray)
	{
		if (PiecesInfoBackup.IsValidIndex(i))
		{
			Piece->SetStatus(PiecesInfoBackup[i].first);
			Piece->SetGridPosition(PiecesInfoBackup[i].second.X, PiecesInfoBackup[i].second.Y);
			i++;
		}
	}
}

/*
 * Function: SeventyFive_MoveRule
 * -----------------------------
 * If the same position occurs five times during the course of the game, the game is automatically a draw.
 * 
 *  Times	int8	Times which the same board configuration has to occured to make the game a draw.
 */
bool AChess_GameMode::SameConfigurationBoard(const int8 Times) const
{
	// ciclo su tutte le configurazioni finché sono uguali, altrimenti break
	int8 Cnt = 0;
	for (const auto& PieceConfiguration : GameSaving)
	{
		int8 ArePiecesEqual = 1;
		int8 i = 0;
		// TArray<> PiecesDoNotMatch
		for (const auto& Piece : PieceConfiguration)
		{
			if (CurrentBoard.IsValidIndex(i))
			{
				if (Piece.Status != CurrentBoard[i].Status
					|| Piece.X != CurrentBoard[i].X
					|| Piece.Y != CurrentBoard[i].Y)
				{
					ArePiecesEqual = 0;
				}
			} 
			else
			{
				int EndPieceConfiguration = PieceConfiguration.Num();
				for (int idx2 = EndPieceConfiguration; idx2 < CurrentBoard.Num(); idx2++)
				{
					if (GField->PawnArray.IsValidIndex(idx2)
						&& GField->PawnArray[idx2]->GetType() == GField->PawnArray[i]->GetType()
						&& GField->PawnArray[idx2]->GetColor() == GField->PawnArray[i]->GetColor()
						&& GField->PawnArray[idx2]->GetStatus() == GField->PawnArray[i]->GetStatus()
						&& Piece.X == CurrentBoard[idx2].X
						&& Piece.Y == CurrentBoard[idx2].Y)
						break; // the two pieces has the same color, type and grid position
					else
						ArePiecesEqual = 0;
				}
			}
			if (ArePiecesEqual == 0)
				break;
			i++;
		}
		Cnt += ArePiecesEqual;
		if (Cnt >= Times)
			break;
	}

	return Cnt >= Times;
}

/*
 * Function: SeventyFive_MoveRule
 * ----------------------------
 * If no capture or no pawn move has occurred in the last 75 moves (by both players), the game is automatically a draw.
 */
bool AChess_GameMode::SeventyFive_MoveRule() const
{
	return (MoveCounter - LastPawnMoveHappened > 75
		|| MoveCounter - LastCaptureHappened > 75);
	
}

/*
 * Function: ImpossibilityToCheckmate
 * ----------------------------
 * Impossibility of checkmate evaluation:
 *	If a position arises in which neither player could possibly give checkmate by a series of legal moves, the game is a draw. 
 *	Such a position is called a dead position. This is usually because there is insufficient material left.
 * Combinations:
 *  - king versus king
 *	- king and bishop versus king
 *  - king and knight versus king
 *  - king and bishop versus king and bishop with the bishops on the same color.
 */
bool AChess_GameMode::ImpossibilityToCheckmate() const
{
	bool Result = true;
	bool KingVsKing = true;
	bool KingBishopVsKing[2] = { true, true }; // 1st: white info, 2nd: black info
	bool KingKnightVsKing[2] = { true, true }; // 1st: white info, 2nd: black info
	bool KingBishopVsKingBishop = true;
	int8 PreviousBishopTileColor[2] = { -1, -1 }; // -1: no bishops | 0: light color | 1: dark color

	for (const auto& Piece : GField->PawnArray)
	{
		if (Piece->GetStatus() == EPawnStatus::ALIVE && TurnPossibleMoves[Piece->GetPieceNum()].Num() > 0)
		{
			int8 ColorIdx = Piece->GetColor() == EPawnColor::WHITE ? 0 : 1;

			if (Piece->GetType() != EPawnType::KING)
				KingVsKing = false;

			if (Piece->GetType() != EPawnType::KNIGHT)
				KingKnightVsKing[0] = false; KingKnightVsKing[1] = false;

			if (Piece->GetType() != EPawnType::BISHOP)
			{
				KingBishopVsKing[0] = false; KingBishopVsKing[1] = false;
				KingBishopVsKingBishop = false;
			}
			else
			{
				if (PreviousBishopTileColor[ColorIdx] == -1)
				{
					int8 tmp = Piece->GetGridPosition()[0] + Piece->GetGridPosition()[1];
					PreviousBishopTileColor[ColorIdx] = tmp % 2 ? 0 : 1;
				}
				else
					KingBishopVsKingBishop = false; // two bishops of the same color

				if (PreviousBishopTileColor[FMath::Abs(ColorIdx - 1)] != -1 
					&& PreviousBishopTileColor[0] != PreviousBishopTileColor[1]) 
					KingBishopVsKingBishop = false; // already analyzed opponent's bishop, it is on different color tile		
			}

			Result = KingVsKing
				|| KingBishopVsKing[0] || KingBishopVsKing[1]
				|| KingKnightVsKing[0] || KingKnightVsKing[1]
				|| KingBishopVsKingBishop;

			if (!Result)
				break;
		}
	}
	
	return Result;
}

/*
 * Function: ReplayMove
 * ----------------------------
 * Loads the turn specified as parameter (e.g. "1. e4" => Replay of the turn number 4.
 *	If user decides to rewind the game (clicking the move N and selecting one of his pieces) at turn N,
 *	the board will be bring back to the status at turn N and the user is allowed to start again the turn N+1
 *
 *	TxtBlock	UTextBlock*		Text block containing the turn to replay (e.g. "1. e4)"
 */
void AChess_GameMode::ReplayMove(UTextBlock* TxtBlock)
{
	// TODO => x le due AI che giocano CurrentPlayer==0 non va bene 
	//	fare Players[CurrentPlayer].IsA(HumanPlayer)
	if (CurrentPlayer == 0)
	{
		// Human is playing (replay available)
		FString BtnName = TxtBlock->GetText().ToString();
		FString NumMoveStr;
		BtnName.Split(TEXT("."), &NumMoveStr, NULL);
		int8 NumMove = FCString::Atoi(*NumMoveStr);
		GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("Replay mossa %d"), NumMove));

		if ((NumMove + 1) != MoveCounter)
		{
			ReplayInProgress = NumMove;
			if (GameSaving.IsValidIndex(NumMove - 1))
			{
				TArray<FPieceSaving> BoardToLoad = GameSaving[NumMove - 1];
				GField->LoadBoard(BoardToLoad);
			}
		}
		else
		{
			// TODO => commento da rimuovere
			GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("Bro cosa stai cercando questo è il turno corrente")));
			GField->LoadBoard(CurrentBoard);

			ReplayInProgress = MoveCounter;
			Players[CurrentPlayer]->OnTurn();
		}
	}
	else
	{
		// AI is playing (replay NOT available)
		GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("BRO, non puoi replayare mentre gioco IO! :)"));
	}
}