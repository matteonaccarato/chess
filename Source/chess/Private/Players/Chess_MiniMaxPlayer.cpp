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
	int8 RandTimer = FMath::Rand() % 21 + 0; // + 10;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
		{
			AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
			if (GameMode && GameMode->ReplayInProgress == 0 && IsMyTurn)
			{



				GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("AI Has %d pawns."), GameMode->BlackPiecesCanMove.Num()));

				// If there are black pawns eligible to move
				if (GameMode->BlackPiecesCanMove.Num() > 0)
				{

					// piece and move X Y
					std::pair<int8, std::pair<int8, int8>> BestMove = FindBestMove(GameMode->GField->TileArray);

					ABasePawn* Pawn = GameMode->GField->PawnArray[BestMove.first];
					int8 OldX = Pawn->GetGridPosition()[0];
					int8 OldY = Pawn->GetGridPosition()[1];
					int8 NewX = BestMove.second.first;
					int8 NewY = BestMove.second.second;

					
					// do the move
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
						GameMode->IsCheck(); // TODO => da rimuoevre in end turn => già fatto prima QUI, o altrimenti inglobo il AddToReplay a EndTurn
						GameMode->AddToReplay(Pawn, EatFlag);
						GameMode->EndTurn(PlayerNumber);
					}
				}
				else
				{
					// No pawns can make eligible moves => BLACK is checkmated
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
	// se re sotto scacco => cosa metto ?
	// se re sotto scacco matto => metto infinito



	return int32();
}

// best_move, max(min)_eval
std::pair<std::pair<int8, std::pair<int8, int8>>, int32> AChess_MiniMaxPlayer::MiniMax(TArray<ATile*> Board, int8 Depth, bool MaximizingPlayer)
{

	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		if (Depth == 0 || GameMode->IsGameOver)
			return std::make_pair(std::make_pair(- 1, std::make_pair(-1, -1)), EvaluateBoard(Board)); // None, Evaluate(board, maximizing color)

		

		TArray<std::pair<int8, TArray<std::pair<int8, int8>>>> PiecesMoves = GameMode->ComputeAllPossibleMoves(EPawnColor::BLACK);

		// moves = compute all current possible moves
		// best_move = random.choice(moves)


		if (MaximizingPlayer)
		{
			int32 max_eval = -1000;

			for (const auto& PieceMove : PiecesMoves)
			{
				for (const auto& Move : PieceMove.second)
				{
					TArray<FTileStatus> TilesStatusesBackup;
					for (auto& Tile : GameMode->GField->TileArray)
					{
						FTileStatus TileStatus = Tile->GetTileStatus();
						TilesStatusesBackup.Add(TileStatus);

						TileStatus.AttackableFrom.SetNum(2, false);
						TileStatus.WhoCanGo.Empty();
						Tile->SetTileStatus(TileStatus);
					}
					int8 XBackup = GameMode->GField->PawnArray[PieceMove.first]->GetGridPosition()[0];
					int8 YBackup = GameMode->GField->PawnArray[PieceMove.first]->GetGridPosition()[1];

					GameMode->MakeMove(GameMode->GField->PawnArray[PieceMove.first], Move.first, Move.second);
					int32 CurrentEval = MiniMax(Board, Depth - 1, false).second;

					// undo the move
					int8 i = 0;
					for (const auto& Tile : GameMode->GField->TileArray)
					{
						Tile->SetTileStatus(TilesStatusesBackup[i]);
						i++;
					}
					GameMode->GField->PawnArray[PieceMove.first]->SetGridPosition(XBackup, YBackup);

					



					/* ... */


				}
			}
				
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
	}

	return std::pair<std::pair<int8, std::pair<int8, int8>>, int32>();
}

std::pair<int8, std::pair<int8, int8>> AChess_MiniMaxPlayer::FindBestMove(TArray<ATile*> Board)
{
	int32 BestVal = -1000;
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

				TArray<FTileStatus> TilesStatusesBackup;
				for (auto& Tile : GameMode->GField->TileArray)
				{
					FTileStatus TileStatus = Tile->GetTileStatus();
					TilesStatusesBackup.Add(TileStatus);

					TileStatus.AttackableFrom.SetNum(2, false);
					TileStatus.WhoCanGo.Empty();
					Tile->SetTileStatus(TileStatus);
				}
				int8 XBackup = GameMode->GField->PawnArray[PieceMove.first]->GetGridPosition()[0];
				int8 YBackup = GameMode->GField->PawnArray[PieceMove.first]->GetGridPosition()[1];


				GameMode->MakeMove(GameMode->GField->PawnArray[PieceMove.first], Move.first, Move.second);

				// compute evaluation function for this move
				// { { piece_num, { newX, newY } }, move_value }
				std::pair<std::pair<int8, std::pair<int8, int8>>, int32> PieceMoveVal = MiniMax(Board, 2, false);

				// undo the move
				int8 i = 0;
				for (const auto& Tile : GameMode->GField->TileArray)
				{
					Tile->SetTileStatus(TilesStatusesBackup[i]);
					i++;
				}
				GameMode->GField->PawnArray[PieceMove.first]->SetGridPosition(XBackup, YBackup);

				// evaluate val
				if (PieceMoveVal.second > BestVal)
				{
					BestMove.first = PieceMoveVal.first.first;
					BestMove.second = PieceMoveVal.first.second;
					BestVal = PieceMoveVal.second;
				}
			}

		}
	}
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("AI (Minimax) bestVal = %d "), BestVal));


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