// Fill out your copyright notice in the Description page of Project Settings.


#include "Players/Chess_MiniMaxPlayer.h"

// Sets default values
AChess_MiniMaxPlayer::AChess_MiniMaxPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GameInstance = Cast<UChess_GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
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


void AChess_MiniMaxPlayer::OnTurn()
{
	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("AI (MiniMax)"));
	if (GameInstance)
		GameInstance->SetTurnMessage(TEXT("AI (MiniMax)"));

	if (GameMode)
	{
		// e.g. RandTimer = 23 => Means a timer of 2.3 seconds
		FTimerHandle TimerHandle;
		int8 RandTimer = GameMode->bIsHumanPlaying ? 
			FMath::Rand() % TIMER_MODULO + TIMER_BASE_OFFSET :
			TIMER_NONE;

		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
			{
				if (IsMyTurn)
				{
					AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
					if (GameMode && GameMode->ReplayInProgress == 0)
					{
						TArray<std::pair<int8, TArray<std::pair<int8, int8>>>>& PlayerPiecesCanMove = Color == EPawnColor::WHITE ? 
							GameMode->WhitePiecesCanMove : 
							GameMode->BlackPiecesCanMove;


						GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("AI Has %d pawns."), PlayerPiecesCanMove.Num()));

						// If there are black pieces eligible to move
						if (PlayerPiecesCanMove.Num() > 0)
						{
							// piece and move X Y
							std::pair<int8, std::pair<int8, int8>> BestMove = FindBestMove(GameMode->GField->TileArray, PlayerPiecesCanMove);

							ABasePawn* Pawn = GameMode->GField->PawnArray[BestMove.first];
							int8 OldX = Pawn->GetGridPosition()[0];
							int8 OldY = Pawn->GetGridPosition()[1];
							int8 NewX = BestMove.second.first;
							int8 NewY = BestMove.second.second;

							// make the move
							bool EatFlag = GameMode->MakeMove(Pawn, NewX, NewY);

							// Pawn promotion handling
							int8 OpponentSide = Color == EPawnColor::WHITE ? GameMode->GField->Size - 1 : 0;
							if (NewX == OpponentSide && Pawn->GetType() == EPawnType::PAWN)
							{
								GameMode->SetPawnPromotionChoice(EPawnType::QUEEN);


								/* int8 RandSpawnPawn = FMath::Rand() % 2;
								switch (RandSpawnPawn)
								{
								case 0: GameMode->SetPawnPromotionChoice(EPawnType::QUEEN); break;
								case 1: GameMode->SetPawnPromotionChoice(EPawnType::KNIGHT); break;
								} */
							}
							else
							{
								// End Turn
								GameMode->LastPiece = Pawn;
								GameMode->LastEatFlag = EatFlag;
								GameMode->EndTurn(PlayerNumber);
							}
						}
						else
						{
							// TODO => rimuovere
							GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("BRO | smth strange happened. u should not be here!"));
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

std::pair<int8, std::pair<int8, int8>> AChess_MiniMaxPlayer::FindBestMove(TArray<ATile*>& Board, TArray<std::pair<int8, TArray<std::pair<int8, int8>>>>& PlayerPieces)
{
	int32 BestVal = -AChess_MiniMaxPlayer::INFINITE;
	std::pair<int8, std::pair<int8, int8>> BestMove;
	BestMove.first = -1; BestMove.second.first = -1; BestMove.second.second = -1;

	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		// for mosse possibili
		// do / undo ciascuna mossa

		// TODO => superfluo ? già fatto a inizio turno => ma deve essere un TArray<TArray<std::pair<int8,int8>>
		// TArray<std::pair<int8, TArray<std::pair<int8, int8>>>> PiecesMoves = GameMode->ComputeAllPossibleMoves(Color);
		for (const auto& PieceMove : PlayerPieces)
		{
			for (const auto& Move : PieceMove.second)
			{
				// Backup
				TArray<FTileStatus> TilesStatusBackup;
				TArray<std::pair<EPawnStatus, FVector2D>> PiecesInfoBackup;
				GameMode->BackupTiles(TilesStatusBackup);
				GameMode->BackupPiecesInfo(PiecesInfoBackup);
				int8 XBackup = GameMode->GField->PawnArray[PieceMove.first]->GetGridPosition()[0];
				int8 YBackup = GameMode->GField->PawnArray[PieceMove.first]->GetGridPosition()[1];
				EPawnStatus PieceStatusBackup = GameMode->GField->PawnArray[PieceMove.first]->GetStatus();
				FCastlingInfo CastlingInfoBackup[2] = { GameMode->CastlingInfoWhite, GameMode->CastlingInfoBlack };
				int8 MaxNumberStepsBackup = GameMode->GField->PawnArray[PieceMove.first]->GetMaxNumberSteps();

				// Make move & compute minimax
				GameMode->MakeMove(GameMode->GField->PawnArray[PieceMove.first], Move.first, Move.second, true);
				// Pawn promotion
				int8 OpponentSide = Color == EPawnColor::WHITE ? GameMode->GField->Size - 1 : 0;
				if (Move.first == OpponentSide && GameMode->GField->PawnArray[PieceMove.first]->GetType() == EPawnType::PAWN)
				{
					GameMode->GField->DespawnPawn(Move.first, Move.second, true);
					GameMode->GField->SpawnPawn(EPawnType::QUEEN, Color, Move.first, Move.second, PlayerNumber, true);
				}

				int32 MoveValue = MiniMax(Board, 2, -AChess_MiniMaxPlayer::INFINITE, AChess_MiniMaxPlayer::INFINITE, Color != EPawnColor::BLACK);

				// undo the move
				GameMode->RestoreTiles(TilesStatusBackup);
				GameMode->RestorePiecesInfo(PiecesInfoBackup);
				GameMode->CastlingInfoWhite = CastlingInfoBackup[0];
				GameMode->CastlingInfoBlack = CastlingInfoBackup[1];
				GameMode->GField->PawnArray[PieceMove.first]->SetGridPosition(XBackup, YBackup);
				GameMode->GField->PawnArray[PieceMove.first]->SetStatus(PieceStatusBackup);
				GameMode->GField->PawnArray[PieceMove.first]->SetMaxNumberSteps(MaxNumberStepsBackup);

				// Compare evaluation
				if (MoveValue > BestVal || (MoveValue == BestVal && FMath::Rand() % PlayerPieces.Num() == 1))
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

// best_move, max(min)_eval
// [ [ piece_num, [to_x, to_y] ], eval ]
int32 AChess_MiniMaxPlayer::MiniMax(TArray<ATile*>& Board, int8 Depth, int32 alpha, int32 beta, bool MaximizingPlayer)
{
	int32 CurrentEval = INFINITE * (MaximizingPlayer ? -1 : 1);

	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		if (Depth == 0 || GameMode->IsGameOver)
			return EvaluateBoard(Board);

		TArray<std::pair<int8, TArray<std::pair<int8, int8>>>> Whites;
		TArray<std::pair<int8, TArray<std::pair<int8, int8>>>> Blacks;
		for (const auto& Piece : GameMode->GField->PawnArray)
		{
			TArray<std::pair<int8, int8>> Tmp = GameMode->ShowPossibleMoves(Piece, false, true, true);
			if (Tmp.Num() > 0)
			{
				switch (Piece->GetColor())
				{
				case EPawnColor::WHITE: Whites.Add(std::make_pair(Piece->GetPieceNum(), Tmp)); break;
				case EPawnColor::BLACK: Blacks.Add(std::make_pair(Piece->GetPieceNum(), Tmp)); break;
				}
			}
		}
		
		EMatchResult Res = GameMode->ComputeMatchResult(Whites, Blacks);
		if (Res != EMatchResult::NONE)
		{
			// TODO 
			GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Yellow, FString::Printf(TEXT("RILEVATO SITUAZIONE BBELLA/BRUTTA")));

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
			// [ piece_num , [ [x1,y1], [x2,y2], ... ], ... ]
			// CurrentEval = -AChess_MiniMaxPlayer::INFINITE;

			for (const auto& PieceMove : Blacks)
			{
				// piece_num , [ [x1,y1], [x2,y2], ... ]
				for (const auto& Move : PieceMove.second)
				{
					// [x1, y1], [x2, y2], ...
				
					// Backup
					TArray<FTileStatus> TilesStatusBackup;
					TArray<std::pair<EPawnStatus, FVector2D>> PiecesInfoBackup;
					GameMode->BackupTiles(TilesStatusBackup);
					GameMode->BackupPiecesInfo(PiecesInfoBackup);
					int8 XBackup = GameMode->GField->PawnArray[PieceMove.first]->GetGridPosition()[0];
					int8 YBackup = GameMode->GField->PawnArray[PieceMove.first]->GetGridPosition()[1];
					EPawnStatus PieceStatusBackup = GameMode->GField->PawnArray[PieceMove.first]->GetStatus();
					FCastlingInfo CastlingInfoBackup[2] = { GameMode->CastlingInfoWhite, GameMode->CastlingInfoBlack };
					int8 MaxNumberStepsBackup = GameMode->GField->PawnArray[PieceMove.first]->GetMaxNumberSteps();

					GameMode->MakeMove(GameMode->GField->PawnArray[PieceMove.first], Move.first, Move.second, true);
					CurrentEval = FMath::Max(CurrentEval, MiniMax(Board, Depth - 1, alpha, beta, !MaximizingPlayer));

					// undo the move
					GameMode->RestoreTiles(TilesStatusBackup);
					GameMode->RestorePiecesInfo(PiecesInfoBackup);
					GameMode->CastlingInfoWhite = CastlingInfoBackup[0];
					GameMode->CastlingInfoBlack = CastlingInfoBackup[1];
					GameMode->GField->PawnArray[PieceMove.first]->SetGridPosition(XBackup, YBackup);
					GameMode->GField->PawnArray[PieceMove.first]->SetStatus(PieceStatusBackup);
					GameMode->GField->PawnArray[PieceMove.first]->SetMaxNumberSteps(MaxNumberStepsBackup); 


					if (CurrentEval >= beta || CurrentEval == AChess_MiniMaxPlayer::INFINITE)
						return CurrentEval; // beta cutoff

					alpha = FMath::Max(alpha, CurrentEval);
				}
			}

			return alpha;
		}
		else
		{
			// [ piece_num , [ [x1,y1], [x2,y2], ... ], ... ]
			/* EMatchResult Res = GameMode->ComputeMatchResult(Whites, Blacks);
			if (Res == EMatchResult::BLACK)
			{
				GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Yellow, FString::Printf(TEXT("Black Checkmate found")));
				return -AChess_MiniMaxPlayer::INFINITE;
			} */

			// CurrentEval = AChess_MiniMaxPlayer::INFINITE;

			for (const auto& PieceMove : Whites)
			{
				// piece_num , [ [x1,y1], [x2,y2], ... ]
				for (const auto& Move : PieceMove.second)
				{
					// [x1, y1], [x2, y2], ...

					// Backup
					TArray<FTileStatus> TilesStatusBackup;
					TArray<std::pair<EPawnStatus, FVector2D>> PiecesInfoBackup;
					GameMode->BackupTiles(TilesStatusBackup);
					GameMode->BackupPiecesInfo(PiecesInfoBackup);
					int8 XBackup = GameMode->GField->PawnArray[PieceMove.first]->GetGridPosition()[0];
					int8 YBackup = GameMode->GField->PawnArray[PieceMove.first]->GetGridPosition()[1];
					EPawnStatus PieceStatusBackup = GameMode->GField->PawnArray[PieceMove.first]->GetStatus();
					FCastlingInfo CastlingInfoBackup[2] = { GameMode->CastlingInfoWhite, GameMode->CastlingInfoBlack };
					int8 MaxNumberStepsBackup = GameMode->GField->PawnArray[PieceMove.first]->GetMaxNumberSteps();

					GameMode->MakeMove(GameMode->GField->PawnArray[PieceMove.first], Move.first, Move.second, true);
					CurrentEval = FMath::Min(CurrentEval, MiniMax(Board, Depth - 1, alpha, beta, !MaximizingPlayer));

					// undo the move
					GameMode->RestoreTiles(TilesStatusBackup);
					GameMode->RestorePiecesInfo(PiecesInfoBackup);
					GameMode->CastlingInfoWhite = CastlingInfoBackup[0];
					GameMode->CastlingInfoBlack = CastlingInfoBackup[1];
					GameMode->GField->PawnArray[PieceMove.first]->SetGridPosition(XBackup, YBackup);
					GameMode->GField->PawnArray[PieceMove.first]->SetStatus(PieceStatusBackup);
					GameMode->GField->PawnArray[PieceMove.first]->SetMaxNumberSteps(MaxNumberStepsBackup);

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


int32 AChess_MiniMaxPlayer::EvaluateBoard(TArray<ATile*> Board) const
{
	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		int Score = 0;
		// int WhiteMobility = 0;
		// int BlackMobility = 0;


		// TODO => non so se lasciarlo o meno
		/* TArray<std::pair<int8, TArray<std::pair<int8, int8>>>> WhiteMoves = GameMode->ComputeAllPossibleMoves(EPawnColor::WHITE, false, true, false);
		TArray<std::pair<int8, TArray<std::pair<int8, int8>>>> BlackMoves = GameMode->ComputeAllPossibleMoves(EPawnColor::BLACK, false, true, false);
		
		TArray<int8> WhitePiecesCanMove;
		TArray<int8> BlackPiecesCanMove;

		bool IsWhite = true;
		for (const auto& Moves : { WhiteMoves, BlackMoves })
		{
			TArray<int8>& PiecesCanMove = IsWhite ? WhitePiecesCanMove : BlackPiecesCanMove;
			for (const auto& Move : Moves)
			{
				PiecesCanMove.Add(Move.first);
			}
			IsWhite = false;
		}
		EMatchResult MatchResult = GameMode->ComputeMatchResult(WhitePiecesCanMove, BlackPiecesCanMove);
		int MatchResultValue = 0;
		switch (MatchResult)
		{
		case EMatchResult::NONE: break;
		case EMatchResult::WHITE: MatchResultValue = 1000; break;
		case EMatchResult::BLACK: MatchResultValue = -1000; break;
		default: MatchResultValue = -50; break;	
		} */

		/* EPawnColor PreviousCheck = GameMode->CheckFlag;
		EPawnColor NewCheck = GameMode->IsCheck();
		GameMode->CheckFlag = PreviousCheck; */
		// TODO => da testare
		/* bool IsWhite = true;
		for (const auto& ColorMoves : { WhiteMoves, BlackMoves })
		{
			for (const auto& Move : ColorMoves)
			{
				int& Mobility = IsWhite ? WhiteMobility : BlackMobility;
				Mobility += Move.second.Num();
			}
			IsWhite = false;
		} */



		GameMode->ComputeAttackableTiles();




		// White, Black
		int8 QueenCounts[2] = { 0, 0 };
		int8 RookCounts[2] = { 0, 0 };
		int8 BishopCounts[2] = { 0, 0 };
		int8 KnightsCounts[2] = { 0, 0 };
		int8 PawnsCounts[2] = { 0, 0 };

		ABasePawn* WhiteKing = GameMode->GField->PawnArray[GameMode->KingWhitePieceNum];
		ABasePawn* BlackKing = GameMode->GField->PawnArray[GameMode->KingBlackPieceNum];
		int8 AttackableKings[2] = {
			GameMode->GField->TileArray[WhiteKing->GetGridPosition()[0] * GameMode->GField->Size + WhiteKing->GetGridPosition()[1]]->GetTileStatus().AttackableFrom[1],
			GameMode->GField->TileArray[BlackKing->GetGridPosition()[0] * GameMode->GField->Size + BlackKing->GetGridPosition()[1]]->GetTileStatus().AttackableFrom[0]
		};
		int BlockingKingsScores[2] = { 
			ComputeBlockingKingScore(BlackKing), // score for white
			ComputeBlockingKingScore(WhiteKing)  // score for black
		};

		// Materials count
		for (const auto& Piece : GameMode->GField->PawnArray)
		{
			if (Piece->GetStatus() == EPawnStatus::ALIVE)
			{
				int8 IdxColor = Piece->GetColor() == EPawnColor::WHITE ? 0 : 1;
				switch (Piece->GetType())
				{
				case EPawnType::QUEEN: QueenCounts[IdxColor]++; break;
				case EPawnType::ROOK: RookCounts[IdxColor]++; break;
				case EPawnType::BISHOP: BishopCounts[IdxColor]++; break;
				case EPawnType::KNIGHT: KnightsCounts[IdxColor]++; break;
				case EPawnType::PAWN: PawnsCounts[IdxColor]++; break;
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

		/* Score = // MatchResultValue
			9 * (QueenCounts[1] - QueenCounts[0])
			+ 8 * ( - static_cast<int8>(NewCheck) ) // BLACK = -1 | WHITE = 1
			+ 5 * (RookCounts[1] - RookCounts[0])
			+ 3 * (BishopCounts[1] - BishopCounts[0])
			+ 3 * (KnightsCounts[1] - KnightsCounts[0])
			+ 1 * (PawnsCounts[1] - PawnsCounts[0]); */
			// + 1 * (BlackMobility - WhiteMobility);
	
		return Score;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode is null"));
		return 0;
	}
}

/*
 * "+" : represents the cell to check if they are attackable from opponent's pieces
 *
 *	+ + +
 * 	+ K +
 *	+ + +
 */
int32 AChess_MiniMaxPlayer::ComputeBlockingKingScore(const ABasePawn* KingToBlock) const
{
	int32 Score = 0;
	int8 OpponentIdx = KingToBlock->GetColor() == EPawnColor::WHITE ? 1 : 0;
	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode && KingToBlock)
	{
		int8 X = KingToBlock->GetGridPosition()[0];
		int8 Y = KingToBlock->GetGridPosition()[1];

		for (int8 VerticalOffset = -1; VerticalOffset <= 1; VerticalOffset++)
		{
			for (int8 HorizontalOffset = -1; HorizontalOffset <= 1; HorizontalOffset++)
			{
				// 2nd condition => to not count if king is directly attackable or not
				if (GameMode->GField->IsValidTile(X + VerticalOffset, Y + HorizontalOffset)
					&& (X + VerticalOffset != 0 || Y + HorizontalOffset != 0))
				{
					if (GameMode->GField->TileArray[(X + VerticalOffset) * GameMode->GField->Size + Y + HorizontalOffset]->GetTileStatus().AttackableFrom[OpponentIdx])
					{
						// TODO test
						// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Yellow, FString::Printf(TEXT("CONTO QUALCOSA DI MAGICO")));
						Score++;
					}
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
		FString Msg = TEXT("AI ");
		if (GameMode->Players.Num() == AChess_GameMode::MIN_NUMBER_SPAWN_PLAYERS) // AI vs AI (Human as spectator)
			Msg += FString::FromInt(PlayerNumber + 1) + " ";
		
		Msg += "WON";
		GameInstance->SetTurnMessage(Msg);

		PlayerNumber ?
			GameInstance->IncrementScorePlayer_2() :
			GameInstance->IncrementScorePlayer_1();
	}
}

void AChess_MiniMaxPlayer::OnLose()
{
	// GameInstance->SetTurnMessage(TEXT("AI Loses"));
}