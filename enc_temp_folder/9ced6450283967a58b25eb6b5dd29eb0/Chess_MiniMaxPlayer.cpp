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
		FTimerHandle TimerHandle;
		// e.g. RandTimer = 23 => Means a timer of 2.3 seconds
		// RandTimer [0.5, 1.0] seconds
		// TODO: sono magic numberss, fare file .ini (o .json) per valori di configurazione (li legge una classe padre, valori statici)
		int8 RandTimer = FMath::Rand() % 5 + 5;

		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
			{
				if (IsMyTurn)
				{
					AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
					if (GameMode && GameMode->ReplayInProgress == 0)
					{

						TArray<int8>& PlayerPiecesCanMove = Color == EPawnColor::WHITE ? GameMode->WhitePiecesCanMove : GameMode->BlackPiecesCanMove;


						GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("AI Has %d pawns."), PlayerPiecesCanMove.Num()));

						// If there are black pieces eligible to move
						if (PlayerPiecesCanMove.Num() > 0)
						{

							// piece and move X Y
							std::pair<int8, std::pair<int8, int8>> BestMove = FindBestMove(GameMode->GField->TileArray);

							ABasePawn* Pawn = GameMode->GField->PawnArray[BestMove.first];
							int8 OldX = Pawn->GetGridPosition()[0];
							int8 OldY = Pawn->GetGridPosition()[1];
							int8 NewX = BestMove.second.first;
							int8 NewY = BestMove.second.second;

							// make the move
							bool EatFlag = GameMode->MakeMove(Pawn, NewX, NewY);

							// Pawn promotion handling
							if (NewX == 0 && Pawn->GetType() == EPawnType::PAWN)
							{
								// Randomically choice of what to promote to
								int8 RandSpawnPawn = FMath::Rand() % 2;
								switch (RandSpawnPawn)
								{
								case 0: GameMode->SetPawnPromotionChoice(EPawnType::QUEEN); break;
								/*  case 1: GameMode->SetPawnPromotionChoice(EPawnType::ROOK); break;
								case 2: GameMode->SetPawnPromotionChoice(EPawnType::BISHOP); break; */
								case 1: GameMode->SetPawnPromotionChoice(EPawnType::KNIGHT); break;
								}
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
							// No pieces can make eligible moves => BLACK is checkmated
							GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("BRO | smth strange happened. u should not be here!"));

							/* GameMode->MatchStatus = EPawnColor::BLACK;
							GameMode->EndTurn(-1); */
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

std::pair<int8, std::pair<int8, int8>> AChess_MiniMaxPlayer::FindBestMove(TArray<ATile*> Board)
{
	int32 BestVal = -10000;
	std::pair<int8, std::pair<int8, int8>> BestMove;
	BestMove.first = -1; BestMove.second.first = -1; BestMove.second.second = -1;

	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		// for mosse possibili
		// do / undo ciascuna mossa

		TArray<std::pair<int8, TArray<std::pair<int8, int8>>>> PiecesMoves = GameMode->ComputeAllPossibleMoves(Color);
		for (const auto& PieceMove : PiecesMoves)
		{
			for (const auto& Move : PieceMove.second)
			{

				TArray<FTileStatus> TilesStatusBackup;
				TArray<std::pair<EPawnStatus, FVector2D>> PiecesInfoBackup;
				GameMode->BackupTiles(TilesStatusBackup);
				GameMode->BackupPiecesInfo(PiecesInfoBackup);
				int8 XBackup = GameMode->GField->PawnArray[PieceMove.first]->GetGridPosition()[0];
				int8 YBackup = GameMode->GField->PawnArray[PieceMove.first]->GetGridPosition()[1];
				EPawnStatus PieceStatusBackup = GameMode->GField->PawnArray[PieceMove.first]->GetStatus();
				FCastlingInfo CastlingInfoBackup[2] = { GameMode->CastlingInfoWhite, GameMode->CastlingInfoBlack };
				int8 MaxNumberStepsBackup = GameMode->GField->PawnArray[PieceMove.first]->GetMaxNumberSteps();

				// GameMode->GField->PawnArray[PieceMove.first]->Move(Move.first, Move.second, true);
				GameMode->MakeMove(GameMode->GField->PawnArray[PieceMove.first], Move.first, Move.second, true);

				// compute evaluation function for this move ( TODO => restituire solo newX newY)
				// { { piece_num, { newX, newY } }, move_value }
				// TODO => 10000s are magic numbers (INFINITEs)
				std::pair<std::pair<int8, std::pair<int8, int8>>, int32> PieceMoveVal = MiniMax(Board, 2, -10000, 10000, Color != EPawnColor::BLACK);

				// undo the move
				GameMode->RestoreTiles(TilesStatusBackup);
				GameMode->RestorePiecesInfo(PiecesInfoBackup);
				GameMode->CastlingInfoWhite = CastlingInfoBackup[0];
				GameMode->CastlingInfoBlack = CastlingInfoBackup[1];
				GameMode->GField->PawnArray[PieceMove.first]->SetGridPosition(XBackup, YBackup);
				GameMode->GField->PawnArray[PieceMove.first]->SetStatus(PieceStatusBackup);
				GameMode->GField->PawnArray[PieceMove.first]->SetMaxNumberSteps(MaxNumberStepsBackup);

				// evaluate val
				if (PieceMoveVal.second > BestVal || (PieceMoveVal.second == BestVal && FMath::Rand() % PiecesMoves.Num() == 1))
				{
					BestMove.first = PieceMove.first;
					BestMove.second.first = Move.first;
					BestMove.second.second = Move.second;
					BestVal = PieceMoveVal.second;
				}
			}

		}
	}

	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("AI (Minimax) bestVal = %d "), BestVal));


	return BestMove;
}

// best_move, max(min)_eval
// [ [ piece_num, [to_x, to_y] ], eval ]
std::pair<std::pair<int8, std::pair<int8, int8>>, int32> AChess_MiniMaxPlayer::MiniMax(TArray<ATile*> Board, int8 Depth, int32 alpha, int32 beta, bool MaximizingPlayer)
{

	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		if (Depth == 0 || GameMode->IsGameOver)
			return std::make_pair(std::make_pair(- 1, std::make_pair(-1, -1)), EvaluateBoard(Board)); // None, Evaluate(board, maximizing color)

		
		

		// moves = compute all current possible moves
		// best_move = random.choice(moves)


		if (MaximizingPlayer)
		{
			// [ piece_num , [ [x1,y1], [x2,y2], ... ], ... ]
			TArray<std::pair<int8, TArray<std::pair<int8, int8>>>> PiecesMoves = GameMode->ComputeAllPossibleMoves(Color);

			int8 MaxPieceNum = -1;
			int8 MaxX = -1;
			int8 MaxY = -1;

			for (const auto& PieceMove : PiecesMoves)
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
					int32 CurrentEval = MiniMax(Board, Depth - 1, alpha, beta, !MaximizingPlayer).second;

					// undo the move
					GameMode->RestoreTiles(TilesStatusBackup);
					GameMode->RestorePiecesInfo(PiecesInfoBackup);
					GameMode->CastlingInfoWhite = CastlingInfoBackup[0];
					GameMode->CastlingInfoBlack = CastlingInfoBackup[1];
					GameMode->GField->PawnArray[PieceMove.first]->SetGridPosition(XBackup, YBackup);
					GameMode->GField->PawnArray[PieceMove.first]->SetStatus(PieceStatusBackup);
					GameMode->GField->PawnArray[PieceMove.first]->SetMaxNumberSteps(MaxNumberStepsBackup); 




					if (CurrentEval >= beta)
						break;

					if (CurrentEval > alpha || (CurrentEval == alpha && FMath::Rand() % PiecesMoves.Num() == 1))
					{
						alpha = CurrentEval;
						MaxPieceNum = PieceMove.first;
						MaxX = Move.first;
						MaxY = Move.second;

						// TODO => magic number (CHECKMATE_VALUE)
						if (CurrentEval == 10000)
						{
							// DA QUALE CICLO ESCO ?
							CurrentEval = CurrentEval;
							break;
						}
					}
				}
			}

			return std::make_pair(
				std::make_pair(
					MaxPieceNum, 
					std::make_pair(MaxX, MaxY)
					), alpha);

				/* board.make_move
				current_eval = minimax(board, depth-1, False)[1]
				board.unmake_move
				if (current_eval > max_eval)
					max_eval = current_eval
					best_move = move
			  return best_move, max_eval */
				

		}
		else
		{
			// [ piece_num , [ [x1,y1], [x2,y2], ... ], ... ]
			TArray<std::pair<int8, TArray<std::pair<int8, int8>>>> PiecesMoves = GameMode->ComputeAllPossibleMoves(Color == EPawnColor::BLACK ? EPawnColor::WHITE : EPawnColor::BLACK);

			int8 MinPieceNum = -1;
			int8 MinX = -1;
			int8 MinY = -1;

			for (const auto& PieceMove : PiecesMoves)
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
					int32 CurrentEval = MiniMax(Board, Depth - 1, alpha, beta, !MaximizingPlayer).second;

					// undo the move
					GameMode->RestoreTiles(TilesStatusBackup);
					GameMode->RestorePiecesInfo(PiecesInfoBackup);
					GameMode->CastlingInfoWhite = CastlingInfoBackup[0];
					GameMode->CastlingInfoBlack = CastlingInfoBackup[1];
					GameMode->GField->PawnArray[PieceMove.first]->SetGridPosition(XBackup, YBackup);
					GameMode->GField->PawnArray[PieceMove.first]->SetStatus(PieceStatusBackup);
					GameMode->GField->PawnArray[PieceMove.first]->SetMaxNumberSteps(MaxNumberStepsBackup);






					if (CurrentEval <= alpha)
						break;

					if (CurrentEval < beta || (CurrentEval == beta && FMath::Rand() % PiecesMoves.Num() == 1))
					{
						beta = CurrentEval;
						MinPieceNum = PieceMove.first;
						MinX = Move.first;
						MinY = Move.second;
						
						// TODO => magic number (CHECKMATE_VALUE)
						if (CurrentEval == -10000)
						{
							// DA QUALE CICLO ESCO ?
							CurrentEval = CurrentEval;
							break;
						}
					}
				}
			}

			return std::make_pair(
				std::make_pair(
					MinPieceNum,
					std::make_pair(MinX, MinY)
				), beta);

			// min_eval = inf

			/* for move : moves
			*	board.make_move
			*	current_eval = minimax(board, depth-1, True)[1]
			*	board.unmake_move
			*	if (current_eval < min_eval)
			*		min_eval = current_eval
			*		best_move = move
			*  return best_move, min_eval
			*/
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode is null"));
		return std::pair<std::pair<int8, std::pair<int8, int8>>, int32>();
	}
}


int32 AChess_MiniMaxPlayer::EvaluateBoard(TArray<ATile*> Board) const
{
	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		// se re sotto scacco matto => metto infinito
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

		Score = 8 * (AttackableKings[0] - AttackableKings[1])
			+ 9 * (QueenCounts[1] - QueenCounts[0])
			+ 5 * (RookCounts[1] - RookCounts[0])
			+ 3 * (BishopCounts[1] - BishopCounts[0])
			+ 3 * (KnightsCounts[1] - KnightsCounts[0])
			+ 1 * (PawnsCounts[1] - PawnsCounts[0]);

		/* Score = // MatchResultValue
			9 * (QueenCounts[1] - QueenCounts[0])
			+ 8 * ( - static_cast<int8>(NewCheck) ) // BLACK = -1 | WHITE = 1
			+ 5 * (RookCounts[1] - RookCounts[0])
			+ 3 * (BishopCounts[1] - BishopCounts[0])
			+ 3 * (KnightsCounts[1] - KnightsCounts[0])
			+ 1 * (PawnsCounts[1] - PawnsCounts[0]); */
			// + 1 * (BlackMobility - WhiteMobility);
		
		// TODO => test
		if (FMath::Abs(Score) == 10000)
		{
			Score = Score;
		}
		return Score;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode is null"));
		return 0;
	}
}


void AChess_MiniMaxPlayer::OnWin()
{
	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode && GameInstance)
	{
		FString Msg = TEXT("AI ");
		if (GameMode->Players.Num() == 3) // ai vs ai
			Msg += FString::FromInt(PlayerNumber);
		
		Msg += "WON";
		GameInstance->SetTurnMessage(Msg);

		PlayerNumber ?
			GameInstance->IncrementScorePlayer_2() :
			GameInstance->IncrementScorePlayer_1();
	}
}

void AChess_MiniMaxPlayer::OnLose()
{
	// TODO
	// GameInstance->SetTurnMessage(TEXT("AI Loses"));
}

void AChess_MiniMaxPlayer::OnDraw()
{
	if (GameInstance)
	{
		GameInstance->SetTurnMessage(TEXT("DRAW"));
	}
}