// Fill out your copyright notice in the Description page of Project Settings.


#include "Players/Chess_MiniMaxPlayer.h"

// Sets default values
AChess_MiniMaxPlayer::AChess_MiniMaxPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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

					int8 OldX = GameMode->GField->PawnArray[BestMove.first]->GetGridPosition()[0];
					int8 OldY = GameMode->GField->PawnArray[BestMove.first]->GetGridPosition()[1];






					// do the move






					int8 NewX = BestMove.second.first;
					int8 NewY = BestMove.second.second;
					bool EatFlag = false;


					// Castling Handling (King moves by two tiles)
					if (GameMode->GField->PawnArray[BestMove.first]->GetType() == EPawnType::KING
						&& FMath::Abs(NewY - OldY) == 2)
					{
						// Move the rook
						bool ShortCastling = (NewY - OldY) > 0;
						int8 RookX = GameMode->GField->PawnArray[BestMove.first]->GetColor() == EPawnColor::WHITE ? 0 : 7;
						int8 OldRookY = ShortCastling ? 7 : 0;
						ATile* OldRookTile = GameMode->GField->TileArray[RookX * GameMode->GField->Size + OldRookY];
						ABasePawn* RookToMove = OldRookTile->GetPawn();

						int8 NewRookY = OldRookY + (ShortCastling ? -2 : 3);
						if (GameMode->GField->IsValidTile(RookX, NewRookY))
						{
							ATile* NewRookTile = GameMode->GField->TileArray[RookX * GameMode->GField->Size + NewRookY];
							if (RookToMove)
							{
								RookToMove->Move(OldRookTile, NewRookTile);
								GameMode->CastlingInfoBlack.KingMoved = true;
								GameMode->CastlingInfoBlack.RooksMoved[NewRookY == 0 ? 0 : 1];
							}
						}
					}

					if (GameMode->GField->PawnArray[BestMove.first]->GetType() == EPawnType::ROOK)
					{
						GameMode->CastlingInfoBlack.RooksMoved[NewY == 0 ? 0 : 1] = true;
					}



					// TODO => superfluo (?, già fatto in gamemode)
					if (GameMode->GField->PawnArray[BestMove.first]->GetType() == EPawnType::PAWN)
					{
						GameMode->GField->PawnArray[BestMove.first]->SetMaxNumberSteps(1);
					}

					// Update last move (useful when doing pawn promotion)
					GameMode->LastGridPosition = FVector2D(NewX, NewY);
					GameMode->PreviousGridPosition = FVector2D(OldX, OldY);
					GameMode->LastEatFlag = EatFlag;// Castling Handling (King moves by two tiles)



					GameMode->IsCheck(); // TODO => da rimuoevre in end turn => già fatto prima QUI, o altrimenti inglobo il AddToReplay a EndTurn
					GameMode->AddToReplay(GameMode->GField->PawnArray[BestMove.first], EatFlag); // TODO => a cosa serve l'eat flag qui se lo segno già in LastEatFlag ?
					GameMode->EndTurn(PlayerNumber);






				}
				else
				{
					// No pawns can make eligible moves => BLACK is checkmated
					GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("AI cannot move anything"));

					GameMode->CheckMateFlag = EPawnColor::BLACK;
					GameMode->EndTurn(-1);
				}



				



				// Setting all tiles as NON attackable from Nobody (TODO => già fatto nell'inizio turno)
				/* for (auto& Tile : GameMode->GField->GetTileArray())
				{
					FTileStatus TileStatus = Tile->GetTileStatus();
					TileStatus.AttackableFrom.SetNum(2, false);
					TileStatus.WhoCanGo.Empty();
					Tile->SetTileStatus(TileStatus);
				}

				GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("AI Has %d pawns."), GameMode->BlackPiecesCanMove.Num()));

				// If there are black pawns eligible to move
				if (GameMode->BlackPiecesCanMove.Num() > 0)
				{
					
				}
				else
				{
					// No pawns can make eligible moves => BLACK is checkmated
					GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("AI cannot move anything"));

					GameMode->CheckMateFlag = EPawnColor::BLACK;
					GameMode->EndTurn(-1); 
				} */
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
std::pair<std::pair<int8, int8>, int32> AChess_MiniMaxPlayer::MiniMax(TArray<ATile*> Board, int8 Depth, bool MaximizingPlayer)
{

	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		if (Depth == 0 || GameMode->IsGameOver)
			return std::make_pair(std::make_pair(-1, -1), EvaluateBoard(Board)); // None, Evaluate(board, maximizing color)

		

		// moves = compute all current possible moves
		// best_move = random.choice(moves)

		if (MaximizingPlayer)
		{
			// max_eval = -inf

			/* for move : moves
			*	board.make_move
			*	current_eval = minimax(board, depth-1, False)[1]
			*	board.unmake_move
			*	if (current_eval > max_eval)
			*		max_eval = current_eval
			*		best_move = move
			*  return best_move, max_eval
			*/	

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

	return std::pair<std::pair<int8, int8>, int32>();
}

std::pair<int8, std::pair<int8, int8>> AChess_MiniMaxPlayer::FindBestMove(TArray<ATile*> Board)
{

	// for mosse possibili
	// do / undo ciascuna mossa



	return std::pair<int8, std::pair<int8, int8>>();
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