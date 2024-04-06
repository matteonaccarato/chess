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
	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("AI (MiniMax)"));
	GameInstance->SetTurnMessage(TEXT("AI (MiniMax)"));

	FTimerHandle TimerHandle;
	// e.g. RandTimer = 23 => Means a timer of 2.3 seconds
	// RandTimer [1.0, 3.0] seconds
	// TODO: sono magic numberss, fare file .ini (o .json) per valori di configurazione (li legge una classe padre, valori statici)
	int8 RandTimer = FMath::Rand() % 5 + 5; // + 10;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
		{
			AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
			if (GameMode && GameMode->ReplayInProgress == 0 && IsMyTurn)
			{



				GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("AI Has %d pawns."), GameMode->BlackPiecesCanMove.Num()));

				// If there are black pieces eligible to move
				if (GameMode->BlackPiecesCanMove.Num() > 0)
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
						int8 RandSpawnPawn = FMath::Rand() % 4;
						switch (RandSpawnPawn)
						{
						case 0: GameMode->SetPawnPromotionChoice(EPawnType::QUEEN); break;
						case 1: GameMode->SetPawnPromotionChoice(EPawnType::ROOK); break;
						case 2: GameMode->SetPawnPromotionChoice(EPawnType::BISHOP); break;
						case 3: GameMode->SetPawnPromotionChoice(EPawnType::KNIGHT); break;
						}
					}
					else
					{
						// End Turn
						GameMode->IsCheck(); // TODO => da rimuovere in end turn => gi� fatto prima QUI, o altrimenti inglobo il AddToReplay a EndTurn
						// GameMode->AddToReplay(Pawn, EatFlag);
						GameMode->LastPiece = Pawn;
						GameMode->LastEatFlag = EatFlag;
						GameMode->EndTurn(PlayerNumber);
					}
				}
				else
				{
					// TODO => else superfluo => gestito in gamemode (tolgo anche da human player )
					// No pieces can make eligible moves => BLACK is checkmated
					GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("AI cannot move anything"));

					GameMode->CheckMateFlag = EPawnColor::BLACK;
					GameMode->EndTurn(-1);
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("GameMode is null"));
			}
		}, RandTimer / 10.f, false);

}

int32 AChess_MiniMaxPlayer::EvaluateBoard(TArray<ATile*> Board) const
{
	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		// se re sotto scacco => cosa metto ?
		// se re sotto scacco matto => metto infinito
		int8 Score = 0;
		int8 WhiteMobility = 0;
		int8 BlackMobility = 0;
		bool WhiteCheckMate = false;
		bool BlackCheckMate = false;
	
		/* f(p) =	200(K - K')
					+ 9(Q - Q')
					+ 5(R - R')
					+ 3(B - B' + N-N')
					+ 1(P - P')
					- 0.5(D - D' + S-S' + I - I')
					+ 0.1(M - M') + ...

			KQRBNP = number of kings, queens, rooks, bishops, knights and pawns
			D, S, I = doubled, blocked and isolated pawns
			M = Mobility(the number of legal moves) */

		/* TArray<std::pair<int8, TArray<std::pair<int8, int8>>>> WhiteMoves = GameMode->ComputeAllPossibleMoves(EPawnColor::WHITE);
		TArray<std::pair<int8, TArray<std::pair<int8, int8>>>> BlackMoves = GameMode->ComputeAllPossibleMoves(EPawnColor::BLACK); 

		if (WhiteMoves.Num() == 0)
			WhiteCheckMate = true;
		if (BlackMoves.Num() == 0)
			BlackCheckMate = true; */

		if (!WhiteCheckMate && !BlackCheckMate)
		{
			// White, Black
			int8 QueenCounts[2] = {0, 0};
			int8 RookCounts[2] = {0, 0};
			int8 BishopCounts[2] = {0, 0};
			int8 KnightsCounts[2] = {0, 0};
			int8 PawnsCounts[2] = {0, 0};

			ABasePawn* WhiteKing = GameMode->GField->PawnArray[GameMode->KingWhitePieceNum];
			ABasePawn* BlackKing = GameMode->GField->PawnArray[GameMode->KingBlackPieceNum];
			int8 AttackableKings[2] = { 
				GameMode->GField->TileArray[WhiteKing->GetGridPosition()[0] * GameMode->GField->Size + WhiteKing->GetGridPosition()[1]]->GetTileStatus().AttackableFrom[1], 
				GameMode->GField->TileArray[BlackKing->GetGridPosition()[0] * GameMode->GField->Size + BlackKing->GetGridPosition()[1]]->GetTileStatus().AttackableFrom[0]
			};

			// TODO => raggruppare i due cicli in funzione
			/* for (const auto& WhiteMove : WhiteMoves)
			{
				WhiteMobility += WhiteMove.second.Num();
			}
			for (const auto& BlackMove : BlackMoves)
			{
				BlackMobility += BlackMove.second.Num();
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


			if (AttackableKings[0])
				GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Yellow, TEXT("White king POTENTIALLY under attack"));
			if (AttackableKings[1]) 
				GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Orange, TEXT("Black king POTENTIALLY under attack"));

			Score = 90 * (AttackableKings[0] - AttackableKings[1])
				+ 9 * (QueenCounts[1] - QueenCounts[0])
				+ 5 * (RookCounts[1] - RookCounts[0])
				+ 3 * (BishopCounts[1] - BishopCounts[0])
				+ 3 * (KnightsCounts[1] - KnightsCounts[0])
				+ 1 * (PawnsCounts[1] - PawnsCounts[0])
				+ 0.1 * (BlackMobility - WhiteMobility);
		}
		else
		{
			Score = WhiteCheckMate ? 1000 : -1000;
		}
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

// best_move, max(min)_eval
// [ [ piece_num, [to_x, to_y] ], eval ]
std::pair<std::pair<int8, std::pair<int8, int8>>, int32> AChess_MiniMaxPlayer::MiniMax(TArray<ATile*> Board, int8 Depth, bool MaximizingPlayer)
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
			TArray<std::pair<int8, TArray<std::pair<int8, int8>>>> PiecesMoves = GameMode->ComputeAllPossibleMoves(EPawnColor::BLACK);

			int32 MaxEval = -10000;
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
					int32 CurrentEval = MiniMax(Board, Depth - 1, false).second;

					// undo the move
					GameMode->RestoreTiles(TilesStatusBackup);
					GameMode->RestorePiecesInfo(PiecesInfoBackup);
					GameMode->CastlingInfoWhite = CastlingInfoBackup[0];
					GameMode->CastlingInfoBlack = CastlingInfoBackup[1];
					GameMode->GField->PawnArray[PieceMove.first]->SetGridPosition(XBackup, YBackup);
					GameMode->GField->PawnArray[PieceMove.first]->SetStatus(PieceStatusBackup);
					GameMode->GField->PawnArray[PieceMove.first]->SetMaxNumberSteps(MaxNumberStepsBackup); 

					if (CurrentEval > MaxEval || (CurrentEval == MaxEval && FMath::Rand() % PiecesMoves.Num() == 1))
					{
						MaxEval = CurrentEval;
						MaxPieceNum = PieceMove.first;
						MaxX = Move.first;
						MaxY = Move.second;

						// TODO => magic number (CHECKMATE_VALUE)
						if (CurrentEval == 1000)
						{
							// DA QUALE CICLO ESCO ?
							break;
						}
					}
					
					/* ... */
				}
			}

			return std::make_pair(
				std::make_pair(
					MaxPieceNum, 
					std::make_pair(MaxX, MaxY)
					), MaxEval);

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
			TArray<std::pair<int8, TArray<std::pair<int8, int8>>>> PiecesMoves = GameMode->ComputeAllPossibleMoves(EPawnColor::WHITE);

			int32 MinEval = 10000;
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
					int32 CurrentEval = MiniMax(Board, Depth - 1, true).second;

					// undo the move
					GameMode->RestoreTiles(TilesStatusBackup);
					GameMode->RestorePiecesInfo(PiecesInfoBackup);
					GameMode->CastlingInfoWhite = CastlingInfoBackup[0];
					GameMode->CastlingInfoBlack = CastlingInfoBackup[1];
					GameMode->GField->PawnArray[PieceMove.first]->SetGridPosition(XBackup, YBackup);
					GameMode->GField->PawnArray[PieceMove.first]->SetStatus(PieceStatusBackup);
					GameMode->GField->PawnArray[PieceMove.first]->SetMaxNumberSteps(MaxNumberStepsBackup);

					if (CurrentEval < MinEval || (CurrentEval == MinEval && FMath::Rand() % PiecesMoves.Num() == 1))
					{
						MinEval = CurrentEval;
						MinPieceNum = PieceMove.first;
						MinX = Move.first;
						MinY = Move.second;
						
						// TODO => magic number (CHECKMATE_VALUE)
						if (CurrentEval == -1000)
						{
							// DA QUALE CICLO ESCO ?
							break;
						}
					}

					/* ... */
				}
			}

			return std::make_pair(
				std::make_pair(
					MinPieceNum,
					std::make_pair(MinX, MinY)
				), MinEval);

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

		TArray<std::pair<int8, TArray<std::pair<int8, int8>>>> PiecesMoves = GameMode->ComputeAllPossibleMoves(EPawnColor::BLACK);
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
				std::pair<std::pair<int8, std::pair<int8, int8>>, int32> PieceMoveVal = MiniMax(Board, 2, true);

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


void AChess_MiniMaxPlayer::OnWin()
{
	// TODO
	GameInstance->SetTurnMessage(TEXT("AI Wins"));
	GameInstance->IncrementScoreAiPlayer();
}

void AChess_MiniMaxPlayer::OnLose()
{
	// TODO
	// GameInstance->SetTurnMessage(TEXT("AI Loses"));
}