// Fill out your copyright notice in the Description page of Project Settings.


#include "Players/Chess_MiniMaxPlayer.h"

// Sets default values
AChess_MiniMaxPlayer::AChess_MiniMaxPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GameInstance = Cast<UChess_GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	EvaluationFunction = EEValuationFunction::BASE;
}

// Called when the game starts or when spawned
void AChess_MiniMaxPlayer::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AChess_MiniMaxPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AChess_MiniMaxPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AChess_MiniMaxPlayer::SetEvaluationFunction(EEValuationFunction Evaluation) { EvaluationFunction = Evaluation; }


void AChess_MiniMaxPlayer::OnTurn()
{
	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, EvaluationFunction == EEValuationFunction::BASE ? UChess_GameInstance::MINIMAX_TURN : UChess_GameInstance::MINIMAX_P);
	if (GameInstance)
		GameInstance->SetTurnMessage(EvaluationFunction == EEValuationFunction::BASE ? UChess_GameInstance::MINIMAX_TURN : UChess_GameInstance::MINIMAX_P);

	if (GameMode)
	{
		// e.g. RandTimer = 23 => means a timer of 2.3 seconds
		FTimerHandle TimerHandle;
		int8 RandTimer = GameMode->bIsHumanPlaying ? 
			FMath::Rand() % TIMER_MODULO + TIMER_BASE_OFFSET :
			TIMER_NONE;

		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
			{
				if (IsMyTurn)
				{
					AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
					// If GameMode is a valid pointer and no replay is showing
					if (GameMode && GameMode->ReplayInProgress == 0)
					{
						TArray<std::pair<int8, TArray<std::pair<int8, int8>>>>& PlayerPiecesCanMove = Color == EPieceColor::WHITE ?
							GameMode->WhitePiecesCanMove :
							GameMode->BlackPiecesCanMove;

						// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("AI Has %d pieces."), PlayerPiecesCanMove.Num()));

						// If there are player's pieces eligible to move
						if (PlayerPiecesCanMove.Num() > 0)
						{
							// Find best move to make ( <piece_number, <new_x, new_y>> )
							std::pair<int8, std::pair<int8, int8>> BestMove = FindBestMove(GameMode->GField->TileArray, PlayerPiecesCanMove);

							if (GameMode->GField->PieceArray.IsValidIndex(BestMove.first))
							{
								ABasePiece* Piece = GameMode->GField->PieceArray[BestMove.first];
								if (Piece)
								{
									int8 OldX = Piece->GetGridPosition()[0];
									int8 OldY = Piece->GetGridPosition()[1];
									int8 NewX = BestMove.second.first;
									int8 NewY = BestMove.second.second;

									// Make the move
									bool EatFlag = GameMode->MakeMove(Piece, NewX, NewY);

									// Pawn promotion handling
									int8 OpponentSide = Color == EPieceColor::WHITE ? GameMode->GField->Size - 1 : 0;
									if (NewX == OpponentSide && Piece->GetType() == EPieceType::PAWN)
									{
										// Queen is the only pawn promotion taken into account since it is the most valuable piece
										GameMode->SetPawnPromotionChoice(EPieceType::QUEEN);
									}
									else
									{
										// End Turn
										GameMode->LastPiece = Piece;
										GameMode->LastEatFlag = EatFlag;
										GameMode->EndTurn(PlayerNumber);
									}
								}
							}
						}
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("GameMode is null"));
					}
				}
			}, RandTimer / 10.f, false);
	}
}

/*
 * Function: FindBestMove
 * ----------------------------
 * Analyse all the possible moves the player can make and choose the best one possible based on the current chess board situation.
 * Its core stands in the MiniMax algorithm implemented in the MiniMax function with the integration of the alpha-beta pruning
 *
 * @param Board			TArray<ATile>*												Current board made of tiles
 * @param PlayerPieces	TArray<std::pair<int8, TArray<std::pair<int8, int8>>>>&		Current player pieces which can move. 
 *																					The structure of a single element of the outer TArray is <piece_number, TArray<new_x, new_y>>
 * 
 * @return				std::pair<int8, std::pair<int8, int8>>						The best move to make where the structure is 
 *																					<piece_number, <new_x, new_y>>
 */
std::pair<int8, std::pair<int8, int8>> AChess_MiniMaxPlayer::FindBestMove(TArray<ATile*>& Board, TArray<std::pair<int8, TArray<std::pair<int8, int8>>>>& PlayerPieces) const
{
	// Return value <piece_number, <new_x, new_y>>
	std::pair<int8, std::pair<int8, int8>> BestMove;
	int32 BestVal = AChess_MiniMaxPlayer::INFINITE * (Color == EPieceColor::BLACK ? -1 : 1);
	BestMove.first = -1; BestMove.second.first = -1; BestMove.second.second = -1;

	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		// Analyse each piece which can be moved
		for (const auto& PieceMove : PlayerPieces)
		{
			// Analyze each piece move
			for (const auto& Move : PieceMove.second)
			{
				// Data backup
				EPieceColor CheckFlagBackup = GameMode->CheckFlag;
				TArray<FTileStatus> TilesStatusBackup;
				TArray<std::pair<EPieceStatus, FVector2D>> PiecesInfoBackup;
				GameMode->GField->BackupTiles(TilesStatusBackup);
				GameMode->GField->BackupPiecesInfo(PiecesInfoBackup);
				int8 XBackup = GameMode->GField->PieceArray[PieceMove.first]->GetGridPosition()[0];
				int8 YBackup = GameMode->GField->PieceArray[PieceMove.first]->GetGridPosition()[1];
				EPieceStatus PieceStatusBackup = GameMode->GField->PieceArray[PieceMove.first]->GetStatus();
				FCastlingInfo CastlingInfoBackup[2] = { GameMode->CastlingInfoWhite, GameMode->CastlingInfoBlack };
				int8 MaxNumberStepsBackup = GameMode->GField->PieceArray[PieceMove.first]->GetMaxNumberSteps();

				// Make the move
				GameMode->MakeMove(GameMode->GField->PieceArray[PieceMove.first], Move.first, Move.second, true);

				// Pawn promotion
				int8 OpponentSide = Color == EPieceColor::WHITE ? GameMode->GField->Size - 1 : 0;
				if (Move.first == OpponentSide && GameMode->GField->PieceArray[PieceMove.first]->GetType() == EPieceType::PAWN)
				{
					GameMode->GField->DespawnPiece(Move.first, Move.second, true);
					GameMode->GField->SpawnPiece(EPieceType::QUEEN, Color, Move.first, Move.second, PlayerNumber, true);
				}

				// MiniMax call
				int32 MoveValue = MiniMax(Board, DEPTH, -AChess_MiniMaxPlayer::INFINITE, AChess_MiniMaxPlayer::INFINITE, Color != EPieceColor::BLACK);

				// Undo the move (restore data)
				GameMode->CheckFlag = CheckFlagBackup;
				GameMode->GField->RestoreTiles(TilesStatusBackup);
				GameMode->GField->RestorePiecesInfo(PiecesInfoBackup);
				GameMode->CastlingInfoWhite = CastlingInfoBackup[0];
				GameMode->CastlingInfoBlack = CastlingInfoBackup[1];
				GameMode->GField->PieceArray[PieceMove.first]->SetGridPosition(XBackup, YBackup);
				GameMode->GField->PieceArray[PieceMove.first]->SetStatus(PieceStatusBackup);
				GameMode->GField->PieceArray[PieceMove.first]->SetMaxNumberSteps(MaxNumberStepsBackup);

				// Compare evaluation
				bool bIsBetterMove = Color == EPieceColor::BLACK ?
					(MoveValue > BestVal) :
					(MoveValue < BestVal);
				if (bIsBetterMove 
					|| PlayerPieces.Num() == 1
					|| (MoveValue == BestVal && FMath::Rand() % PlayerPieces.Num() == 1))
				{
					BestMove.first = PieceMove.first;
					BestMove.second.first = Move.first;
					BestMove.second.second = Move.second;
					BestVal = MoveValue;

					if (BestVal == AChess_MiniMaxPlayer::INFINITE * -static_cast<int>(Color))
					{
						GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("AI (Minimax) bestVal = %d "), BestVal));
						return BestMove;
					}
				}
			}

		}
	}

	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("AI (Minimax) bestVal = %d "), BestVal));

	return BestMove;
}


/*
 * Function: MiniMax (with alpha-beta pruning)
 * ----------------------------
 * Analyse all the possible moves the player can make and choose the best one possible based on the current chess board situation.
 * The black player has to maximize the board evaluation, while the white one has to minimize it
 *
 * @param Board				TArray<ATile>*	Current board made of tiles
 * @param Depth				int8			Depth of the minimax algorithm
 * @param Alpha				int32			Store the alpha value to allow alpha-beta pruning implentation to improve time complexity
 * @param Beta				int32			Store the beta value to allow alpha-beta pruning implentation to improve time complexity
 * @param MaximizingPlayer	bool			White -> True
 *											Black -> False
 * 
 * @return			int32			Best board evaluation
 */
int32 AChess_MiniMaxPlayer::MiniMax(TArray<ATile*>& Board, int8 Depth, int32 alpha, int32 beta, bool MaximizingPlayer) const
{
	// Initialise evaluation
	int32 CurrentEval = INFINITE * (MaximizingPlayer ? -1 : 1);

	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		if (Depth == 0 || GameMode->IsGameOver)
			return EvaluateBoard();

		// Compute all possible moves
		TArray<std::pair<int8, TArray<std::pair<int8, int8>>>> Whites;
		TArray<std::pair<int8, TArray<std::pair<int8, int8>>>> Blacks;
		for (const auto& Piece : GameMode->GField->PieceArray)
		{
			TArray<std::pair<int8, int8>> Tmp = GameMode->ShowPossibleMoves(Piece, false, true, true);
			if (Tmp.Num() > 0)
			{
				switch (Piece->GetColor())
				{
				case EPieceColor::WHITE: Whites.Add(std::make_pair(Piece->GetPieceNum(), Tmp)); break;
				case EPieceColor::BLACK: Blacks.Add(std::make_pair(Piece->GetPieceNum(), Tmp)); break;
				}
			}
		}
		
		if (GameMode->MatchStatus == EMatchResult::NONE)
			GameMode->CheckFlag = GameMode->CheckKingsUnderAttack();
		EMatchResult Res = GameMode->ComputeMatchResult(Whites, Blacks);
		if (Res != EMatchResult::NONE)
		{
			int8  MaximizingPlayerSign = MaximizingPlayer ? 1 : -1;
			EMatchResult GoodSituation = MaximizingPlayer ? EMatchResult::WHITE : EMatchResult::BLACK;
			
			// +1 => good situation (opponent is checkmated)
			// -1 => bad situation	(current player checkmated or draw)
			int8 Sign = Res == GoodSituation ? 1 : -1;
			// return (AChess_MiniMaxPlayer::INFINITE * Sign * MaximizingPlayerSign) / 2;
			int32 BaseValue = (Res == EMatchResult::WHITE || Res == EMatchResult::BLACK) ?
				AChess_MiniMaxPlayer::INFINITE :
				AChess_MiniMaxPlayer::INFINITE / 2;
			CurrentEval = (BaseValue * Sign * MaximizingPlayerSign);
		}

		if (MaximizingPlayer)
		{
			// Black player
			// Analyse each piece 
			for (const auto& PieceMove : Blacks)
			{
				// Analyse each piece move <new_x, new_y>
				for (const auto& Move : PieceMove.second)
				{
					// Data backup
					EPieceColor CheckFlagBackup = GameMode->CheckFlag;
					TArray<FTileStatus> TilesStatusBackup;
					TArray<std::pair<EPieceStatus, FVector2D>> PiecesInfoBackup;
					GameMode->GField->BackupTiles(TilesStatusBackup);
					GameMode->GField->BackupPiecesInfo(PiecesInfoBackup);
					int8 XBackup = GameMode->GField->PieceArray[PieceMove.first]->GetGridPosition()[0];
					int8 YBackup = GameMode->GField->PieceArray[PieceMove.first]->GetGridPosition()[1];
					EPieceStatus PieceStatusBackup = GameMode->GField->PieceArray[PieceMove.first]->GetStatus();
					FCastlingInfo CastlingInfoBackup[2] = { GameMode->CastlingInfoWhite, GameMode->CastlingInfoBlack };
					int8 MaxNumberStepsBackup = GameMode->GField->PieceArray[PieceMove.first]->GetMaxNumberSteps();

					// Make the move & compute new minimax at depth - 1
					GameMode->MakeMove(GameMode->GField->PieceArray[PieceMove.first], Move.first, Move.second, true);
					CurrentEval = FMath::Max(CurrentEval, MiniMax(Board, Depth - 1, alpha, beta, !MaximizingPlayer));

					// Undo the move (restore data)
					GameMode->CheckFlag = CheckFlagBackup;
					GameMode->GField->RestoreTiles(TilesStatusBackup);
					GameMode->GField->RestorePiecesInfo(PiecesInfoBackup);
					GameMode->CastlingInfoWhite = CastlingInfoBackup[0];
					GameMode->CastlingInfoBlack = CastlingInfoBackup[1];
					GameMode->GField->PieceArray[PieceMove.first]->SetGridPosition(XBackup, YBackup);
					GameMode->GField->PieceArray[PieceMove.first]->SetStatus(PieceStatusBackup);
					GameMode->GField->PieceArray[PieceMove.first]->SetMaxNumberSteps(MaxNumberStepsBackup); 

					// Compare values
					if (CurrentEval >= beta || CurrentEval == AChess_MiniMaxPlayer::INFINITE)
						return CurrentEval; // beta cutoff

					alpha = FMath::Max(alpha, CurrentEval);
				}
			}

			return alpha;
		}
		else
		{
			// White player
			// Analyse each piece
			for (const auto& PieceMove : Whites)
			{
				// Analyse each piece move <new_x, new_y>
				for (const auto& Move : PieceMove.second)
				{
					// Data backup
					EPieceColor CheckFlagBackup = GameMode->CheckFlag;
					TArray<FTileStatus> TilesStatusBackup;
					TArray<std::pair<EPieceStatus, FVector2D>> PiecesInfoBackup;
					GameMode->GField->BackupTiles(TilesStatusBackup);
					GameMode->GField->BackupPiecesInfo(PiecesInfoBackup);
					int8 XBackup = GameMode->GField->PieceArray[PieceMove.first]->GetGridPosition()[0];
					int8 YBackup = GameMode->GField->PieceArray[PieceMove.first]->GetGridPosition()[1];
					EPieceStatus PieceStatusBackup = GameMode->GField->PieceArray[PieceMove.first]->GetStatus();
					FCastlingInfo CastlingInfoBackup[2] = { GameMode->CastlingInfoWhite, GameMode->CastlingInfoBlack };
					int8 MaxNumberStepsBackup = GameMode->GField->PieceArray[PieceMove.first]->GetMaxNumberSteps();

					// Make the move & compute minimax with depth - 1
					GameMode->MakeMove(GameMode->GField->PieceArray[PieceMove.first], Move.first, Move.second, true);
					CurrentEval = FMath::Min(CurrentEval, MiniMax(Board, Depth - 1, alpha, beta, !MaximizingPlayer));

					// Undo the move (restore data)
					GameMode->CheckFlag = CheckFlagBackup;
					GameMode->GField->RestoreTiles(TilesStatusBackup);
					GameMode->GField->RestorePiecesInfo(PiecesInfoBackup);
					GameMode->CastlingInfoWhite = CastlingInfoBackup[0];
					GameMode->CastlingInfoBlack = CastlingInfoBackup[1];
					GameMode->GField->PieceArray[PieceMove.first]->SetGridPosition(XBackup, YBackup);
					GameMode->GField->PieceArray[PieceMove.first]->SetStatus(PieceStatusBackup);
					GameMode->GField->PieceArray[PieceMove.first]->SetMaxNumberSteps(MaxNumberStepsBackup);

					// Compare values
					if (CurrentEval <= alpha || CurrentEval == -AChess_MiniMaxPlayer::INFINITE)
						return CurrentEval; // alpha cutoff

					beta = FMath::Min(beta, CurrentEval);
				}
			}

			return beta;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode is null"));
		return 0;
	}
}


/*
 * Function: EvaluationBoard
 * ----------------------------
 * Evaluate the current board situation through function f defined as follows:
 * f = QUEEN_VALUE * (Q' - Q)
 *		+ ATTACKABLE_KING_VALUE * (AK - AK')
 *		+ BLOCKING_KING_VALUE * (BK' - BK)
 *		+ ROOK_VALUE * (R' - R)
 * 		+ BISHOP_VALUE * (B' - B)
 *		+ KNIGHT_VALUE * (N' - N)
 *		+ PAWN_VALUE * (P' - P)
 * The prime value (e.g. X') means the number of pieces of type X of the black player,
 * while the standard value (e.g. X) indicates the one of the white player
 *
 * @param Board		TArray<ATile>*	Current board made of tiles
 * 
 * @return			int32			Board evaluation
 */





int32 AChess_MiniMaxPlayer::Base() const 
{
	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		int Score = 0;

		// White, Black score variables
		int8 QueenCounts[2] = { 0, 0 };
		int8 RookCounts[2] = { 0, 0 };
		int8 BishopCounts[2] = { 0, 0 };
		int8 KnightsCounts[2] = { 0, 0 };
		int8 PawnsCounts[2] = { 0, 0 };

		GameMode->ComputeAttackableTiles();
		ABasePiece* WhiteKing = GameMode->GField->PieceArray[GameMode->KingWhitePieceNum];
		ABasePiece* BlackKing = GameMode->GField->PieceArray[GameMode->KingBlackPieceNum];
		int8 AttackableKings[2] = {
			GameMode->GField->TileArray[WhiteKing->GetGridPosition()[0] * GameMode->GField->Size + WhiteKing->GetGridPosition()[1]]->GetTileStatus().AttackableFrom[1],
			GameMode->GField->TileArray[BlackKing->GetGridPosition()[0] * GameMode->GField->Size + BlackKing->GetGridPosition()[1]]->GetTileStatus().AttackableFrom[0]
		};
		int BlockingKingsScores[2] = {
			ComputeBlockingKingScore(BlackKing), // score for white
			ComputeBlockingKingScore(WhiteKing)  // score for black
		};

		// Materials count
		for (const auto& Piece : GameMode->GField->PieceArray)
		{
			if (Piece->GetStatus() == EPieceStatus::ALIVE)
			{
				int8 IdxColor = Piece->GetColor() == EPieceColor::WHITE ? 0 : 1;
				switch (Piece->GetType())
				{
				case EPieceType::QUEEN: QueenCounts[IdxColor]++; break;
				case EPieceType::ROOK: RookCounts[IdxColor]++; break;
				case EPieceType::BISHOP: BishopCounts[IdxColor]++; break;
				case EPieceType::KNIGHT: KnightsCounts[IdxColor]++; break;
				case EPieceType::PAWN: PawnsCounts[IdxColor]++; break;
				}
			}
		}

		Score = AChess_MiniMaxPlayer::QUEEN_VALUE * (QueenCounts[1] - QueenCounts[0])
			+ AChess_MiniMaxPlayer::ATTACKABLE_KING_VALUE * (AttackableKings[0] - AttackableKings[1])
			+ AChess_MiniMaxPlayer::BLOCKING_KING_VALUE * (BlockingKingsScores[1] - BlockingKingsScores[0])
			+ AChess_MiniMaxPlayer::ROOK_VALUE * (RookCounts[1] - RookCounts[0])
			+ AChess_MiniMaxPlayer::BISHOP_VALUE * (BishopCounts[1] - BishopCounts[0])
			+ AChess_MiniMaxPlayer::KNIGHT_VALUE * (KnightsCounts[1] - KnightsCounts[0])
			+ AChess_MiniMaxPlayer::PAWN_VALUE * (PawnsCounts[1] - PawnsCounts[0]);

		return Score;

	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode is null"));
		return 0;
	}
}



int32 AChess_MiniMaxPlayer::GetPieceValueByMaterial(const ABasePiece* Piece, const EGamePhase GamePhase) const
{
	// pawn , knight , bishop , rook , queen
	const TArray<int>& MaterialsWeights = GamePhase == EGamePhase::OPENING ?
		AChess_MiniMaxPlayer::OpeningMaterialWeights :
		AChess_MiniMaxPlayer::EndgameMaterialWeights;
		
	int Score = 0;
	switch (Piece->GetType())
	{
	case EPieceType::PAWN: Score = MaterialsWeights[0]; break;
	case EPieceType::KNIGHT: Score = MaterialsWeights[1]; break;
	case EPieceType::BISHOP: Score = MaterialsWeights[2]; break;
	case EPieceType::ROOK: Score = MaterialsWeights[3]; break;
	case EPieceType::QUEEN: Score = MaterialsWeights[4]; break;
	default: Score = 0;
	}

	return Score; // * static_cast<int>(Piece->GetColor());
}

int32 AChess_MiniMaxPlayer::GetPieceValueByPosition(const ABasePiece* Piece, const EGamePhase GamePhase) const
{
	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		int Score = 0;
		// Compute table X,Y by piece color
		int X = Piece->GetColor() == EPieceColor::WHITE ?
			Piece->GetGridPosition()[0] :
			(GameMode->GField->Size - Piece->GetGridPosition()[0]);
		int Y = Piece->GetColor() == EPieceColor::WHITE ?
			Piece->GetGridPosition()[1] :
			(GameMode->GField->Size - Piece->GetGridPosition()[1]);

		if (GameMode->GField->IsValidTile(X, Y))
		{
			switch (GamePhase)
			{
			case EGamePhase::MIDDLEGAME:
				switch (Piece->GetType())
				{
					case EPieceType::PAWN: Score = MG_PawnTable[X*GameMode->GField->Size + Y];
					case EPieceType::KNIGHT: Score = MG_KnightTable[X * GameMode->GField->Size + Y];
					case EPieceType::BISHOP: Score = MG_BishopTable[X * GameMode->GField->Size + Y];
					case EPieceType::ROOK: Score = MG_RookTable[X * GameMode->GField->Size + Y];
					case EPieceType::QUEEN: Score = MG_QueenTable[X * GameMode->GField->Size + Y];
					case EPieceType::KING: Score = MG_KingTable[X * GameMode->GField->Size + Y];
				}
		
			case EGamePhase::ENDGAME:
				switch (Piece->GetType())
				{
				case EPieceType::PAWN: Score = EG_PawnTable[X * GameMode->GField->Size + Y];
				case EPieceType::KNIGHT: Score = EG_KnightTable[X * GameMode->GField->Size + Y];
				case EPieceType::BISHOP: Score = EG_BishopTable[X * GameMode->GField->Size + Y];
				case EPieceType::ROOK: Score = EG_RookTable[X * GameMode->GField->Size + Y];
				case EPieceType::QUEEN: Score = EG_QueenTable[X * GameMode->GField->Size + Y];
				case EPieceType::KING: Score = EG_KingTable[X * GameMode->GField->Size + Y];
				}
			}
		}
		return Score; // * static_cast<int>(Piece->GetColor());;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode is null"));
		return 0;
	}
}

int AChess_MiniMaxPlayer::GetGamePhaseScore() const
{
	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		int GamePhaseScore = 0;
		int8 QueenCounts[2] = { 0, 0 };
		int8 RookCounts[2] = { 0, 0 };
		int8 BishopCounts[2] = { 0, 0 };
		int8 KnightsCounts[2] = { 0, 0 };
		int8 PawnsCounts[2] = { 0, 0 };
		for (const auto& Piece : GameMode->GField->PieceArray)
		{
			if (Piece->GetStatus() == EPieceStatus::ALIVE)
			{
				int8 IdxColor = Piece->GetColor() == EPieceColor::WHITE ? 0 : 1;
				switch (Piece->GetType())
				{
				case EPieceType::QUEEN: GamePhaseScore += OpeningMaterialWeights[5]; break;
				case EPieceType::ROOK: GamePhaseScore += OpeningMaterialWeights[4];  break;
				case EPieceType::BISHOP: GamePhaseScore += OpeningMaterialWeights[2];  break;
				case EPieceType::KNIGHT: GamePhaseScore += OpeningMaterialWeights[1];  break;
				case EPieceType::PAWN: GamePhaseScore += OpeningMaterialWeights[0];  break;
				}
			}
		}

		return GamePhaseScore;
	}

	return 0;
}


/*
* =============================V2======================================
* // Funzione per determinare la fase del gioco (es. soglia semplificata)
bool isEndgame(const int8 QueenCounts[2], const int8 RookCounts[2], const int8 BishopCounts[2], const int8 KnightsCounts[2]) {
    int materialWeight = 0;
    materialWeight += (QueenCounts[0] + QueenCounts[1]) * 9;
    materialWeight += (RookCounts[0] + RookCounts[1]) * 5;
    materialWeight += (BishopCounts[0] + BishopCounts[1]) * 3;
    materialWeight += (KnightsCounts[0] + KnightsCounts[1]) * 3;
    // Consideriamo anche i pedoni (aggiungi se necessario)

    return materialWeight < 14; // Soglia per considerare endgame
}

int EvaluateBoard(const GameMode* GameMode) {
    int Score = 0;

    // Variabili di punteggio per bianco e nero
    int8 QueenCounts[2] = { 0, 0 };
    int8 RookCounts[2] = { 0, 0 };
    int8 BishopCounts[2] = { 0, 0 };
    int8 KnightsCounts[2] = { 0, 0 };
    int8 PawnsCounts[2] = { 0, 0 };

    GameMode->ComputeAttackableTiles();
    ABasePiece* WhiteKing = GameMode->GField->PieceArray[GameMode->KingWhitePieceNum];
    ABasePiece* BlackKing = GameMode->GField->PieceArray[GameMode->KingBlackPieceNum];
    int8 AttackableKings[2] = {
        GameMode->GField->TileArray[WhiteKing->GetGridPosition()[0] * GameMode->GField->Size + WhiteKing->GetGridPosition()[1]]->GetTileStatus().AttackableFrom[1],
        GameMode->GField->TileArray[BlackKing->GetGridPosition()[0] * GameMode->GField->Size + BlackKing->GetGridPosition()[1]]->GetTileStatus().AttackableFrom[0]
    };
    int BlockingKingsScores[2] = {
        ComputeBlockingKingScore(BlackKing), // score for white
        ComputeBlockingKingScore(WhiteKing)  // score for black
    };

    // Conteggio dei materiali
    for (const auto& Piece : GameMode->GField->PieceArray) {
        if (Piece->GetStatus() == EPieceStatus::ALIVE) {
            int8 IdxColor = Piece->GetColor() == EPieceColor::WHITE ? 0 : 1;
            switch (Piece->GetType()) {
                case EPieceType::QUEEN: QueenCounts[IdxColor]++; break;
                case EPieceType::ROOK: RookCounts[IdxColor]++; break;
                case EPieceType::BISHOP: BishopCounts[IdxColor]++; break;
                case EPieceType::KNIGHT: KnightsCounts[IdxColor]++; break;
                case EPieceType::PAWN: PawnsCounts[IdxColor]++; break;
            }
        }
    }

    // Determina se siamo in endgame
    bool endgame = isEndgame(QueenCounts, RookCounts, BishopCounts, KnightsCounts);

    // Calcolo del punteggio del materiale e delle Piece Square Tables
    for (const auto& Piece : GameMode->GField->PieceArray) {
        if (Piece->GetStatus() == EPieceStatus::ALIVE) {
            int8 IdxColor = Piece->GetColor() == EPieceColor::WHITE ? 0 : 1;
            int position = Piece->GetGridPosition()[0] * GameMode->GField->Size + Piece->GetGridPosition()[1];
            if (!Piece->GetColor() == EPieceColor::WHITE) {
                position = 63 - position; // Inverti la posizione per i neri
            }
            
            int pieceSquareValue = 0;
            switch (Piece->GetType()) {
                case EPieceType::PAWN:
                    pieceSquareValue = endgame ? eg_pawn_table[position] : mg_pawn_table[position];
                    break;
                case EPieceType::KNIGHT:
                    pieceSquareValue = endgame ? eg_knight_table[position] : mg_knight_table[position];
                    break;
                // Aggiungi casi per bishop, rook, queen, king
            }

            Score += Piece->GetColor() == EPieceColor::WHITE ? pieceSquareValue : -pieceSquareValue;
        }
    }

    // Calcolo del punteggio totale
    Score += AChess_MiniMaxPlayer::QUEEN_VALUE * (QueenCounts[1] - QueenCounts[0])
            + AChess_MiniMaxPlayer::ATTACKABLE_KING_VALUE * (AttackableKings[0] - AttackableKings[1])
            + AChess_MiniMaxPlayer::BLOCKING_KING_VALUE * (BlockingKingsScores[1] - BlockingKingsScores[0])
            + AChess_MiniMaxPlayer::ROOK_VALUE * (RookCounts[1] - RookCounts[0])
            + AChess_MiniMaxPlayer::BISHOP_VALUE * (BishopCounts[1] - BishopCounts[0])
            + AChess_MiniMaxPlayer::KNIGHT_VALUE * (KnightsCounts[1] - KnightsCounts[0])
            + AChess_MiniMaxPlayer::PAWN_VALUE * (PawnsCounts[1] - PawnsCounts[0]);

    return Score;
}==========================================================================
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* ==================================V1================================
* 
int evaluateMaterial(const std::vector<Piece>& pieces) {
	int score = 0;

	for (const Piece& piece : pieces) {
		int value = 0;
		switch (piece.type) {
			case PAWN: value = PAWN; break;
			case KNIGHT: value = KNIGHT; break;
			case BISHOP: value = BISHOP; break;
			case ROOK: value = ROOK; break;
			case QUEEN: value = QUEEN; break;
			case KING: value = KING; break;
			default: value = 0; break;
		}
		score += piece.isWhite ? value : -value;
	}

	return score;
}



int evaluatePieceSquareTables(const std::vector<Piece>& pieces, bool isEndgame) {
	int score = 0;

	for (const Piece& piece : pieces) {
		int value = 0;
		int position = piece.position;
		if (!piece.isWhite) {
			position = 63 - position; // Inverti la posizione per i neri
		}

		switch (piece.type) {
			case PAWN: value = isEndgame ? eg_pawn_table[position] : mg_pawn_table[position]; break;
			case KNIGHT: value = isEndgame ? eg_knight_table[position] : mg_knight_table[position]; break;
			// Altri casi per bishop, rook, queen, king
		}
		score += piece.isWhite ? value : -value;
	}

	return score;
}

bool isEndgame(const std::vector<Piece>& pieces) {
	int materialWeight = 0;
	for (const Piece& piece : pieces) {
		switch (piece.type) {
			case PAWN: materialWeight += PAWN; break;
			case KNIGHT: materialWeight += KNIGHT; break;
			case BISHOP: materialWeight += BISHOP; break;
			case ROOK: materialWeight += ROOK; break;
			case QUEEN: materialWeight += QUEEN; break;
			default: break;
		}
	}
	return materialWeight < 1400; // Soglia per considerare endgame
}


int evaluate(const std::vector<Piece>& pieces) {
	int materialScore = evaluateMaterial(pieces);
	bool endgame = isEndgame(pieces);
	int pieceSquareTableScore = evaluatePieceSquareTables(pieces, endgame);

	return materialScore + pieceSquareTableScore;
}

*/


// TODO => non serve la board
int32 AChess_MiniMaxPlayer::Pesto() const
{


	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		int Score = Base(); // base evaluation
		int Score2 = 0;


		// for pieces add/remove score based on piece position
		for (const auto& Piece : GameMode->GField->PieceArray)
		{
			if (Piece->GetStatus() == EPieceStatus::ALIVE)
			{
				// TODO => check if middle or end game
				Score2 += GetPieceValueByPosition(Piece, EGamePhase::MIDDLEGAME);
				
			}
		}



		/*
		
		int materialScore = evaluateMaterial(pieces);
		bool endgame = isEndgame(pieces);
		int pieceSquareTableScore = evaluatePieceSquareTables(pieces, endgame);

		return materialScore + pieceSquareTableScore;
		
		*/





		GEngine->AddOnScreenDebugMessage(-1, 20.f, 
			FColor::Green, 
			FString::Printf(TEXT("PESTO %d %d "), Score, Score2));
		return Score + Score2;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode is null"));
		return 0;
	}



}











int32 AChess_MiniMaxPlayer::EvaluateBoard() const
{
	switch (EvaluationFunction)
	{
	case EEValuationFunction::BASE: return Base();
	case EEValuationFunction::PESTO: return Pesto();
	default: return 0;
	}
}


/*
 * Function: ComputeBlockingKingScore
 * ----------------------------
 * Compute if the king is blocked by opponent moves in some directions.
 * "+" : represents the cell to check if they are attackable from opponent's pieces
 *
 *	+ + +
 * 	+ K +
 *	+ + +
 *
 * @param KingToBlock	const ABasePiece*	Pointer to the king to block the moves to
 *
 * @return				int32				Number of blocked directions  
 *
 */
int32 AChess_MiniMaxPlayer::ComputeBlockingKingScore(const ABasePiece* KingToBlock) const
{
	int32 Score = 0;
	int8 OpponentIdx = KingToBlock->GetColor() == EPieceColor::WHITE ? 1 : 0;
	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode && KingToBlock)
	{
		int8 X = KingToBlock->GetGridPosition()[0];
		int8 Y = KingToBlock->GetGridPosition()[1];

		for (int8 VerticalOffset = -1; VerticalOffset <= 1; VerticalOffset++)
		{
			for (int8 HorizontalOffset = -1; HorizontalOffset <= 1; HorizontalOffset++)
			{
				// 2nd condition => to not count if king is directly attackable or not (this means king is under check)
				if (GameMode->GField->IsValidTile(X + VerticalOffset, Y + HorizontalOffset)
					&& (X + VerticalOffset != 0 || Y + HorizontalOffset != 0))
				{
					if (GameMode->GField->TileArray[(X + VerticalOffset) * GameMode->GField->Size + Y + HorizontalOffset]->GetTileStatus().AttackableFrom[OpponentIdx])
						Score++;
				}
			}
		}
	}

	return Score;
}


void AChess_MiniMaxPlayer::OnWin()
{
	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode && GameInstance)
	{
		FString Msg = UChess_GameInstance::MINIMAX_WIN_1;
		if (GameMode->Players.Num() == AChess_GameMode::MIN_NUMBER_SPAWN_PLAYERS) // AI vs AI (Human as spectator)
			Msg += FString::FromInt(PlayerNumber + 1) + " ";
		
		Msg += UChess_GameInstance::MINIMAX_WIN_2;
		GameInstance->SetTurnMessage(Msg);

		PlayerNumber ?
			GameInstance->IncrementScorePlayer_2() :
			GameInstance->IncrementScorePlayer_1();
	}
}

void AChess_MiniMaxPlayer::OnLose()
{
	// GameInstance->SetTurnMessage(UChess_GameInstance::MINIMAX_DEFEAT);
}