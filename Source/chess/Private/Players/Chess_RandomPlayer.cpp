// Fill out your copyright notice in the Description page of Project Settings.


#include "Players/Chess_RandomPlayer.h"

// Sets default values
AChess_RandomPlayer::AChess_RandomPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	GameInstance = Cast<UChess_GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
}

// Called when the game starts or when spawned
void AChess_RandomPlayer::BeginPlay()
{
	Super::BeginPlay();	
}

// Called every frame
/* void AChess_RandomPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
} */

// Called to bind functionality to input
void AChess_RandomPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}


void AChess_RandomPlayer::OnTurn()
{
	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, UChess_GameInstance::RANDOM_TURN);
	if (GameInstance)
		GameInstance->SetTurnMessage(UChess_GameInstance::RANDOM_TURN);

	if (GameMode)
	{
		FTimerHandle TimerHandle;
		// e.g. RandTimer = 23 => means a timer of 2.3 seconds
		int8 RandTimer = GameMode->bIsHumanPlaying ? FMath::Rand() % TIMER_MODULO + TIMER_BASE_OFFSET : TIMER_NONE;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
			{
				if (IsMyTurn)
				{
					AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
					// If GameMode is a valid pointer and no replay is showing
					if (GameMode && GameMode->ReplayInProgress == 0)
					{
						TArray<std::pair<int8, TArray<std::pair<int8, int8>>>>& PlayerPiecesCanMove = Color == EPawnColor::WHITE ?
							GameMode->WhitePiecesCanMove : 
							GameMode->BlackPiecesCanMove;

						// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("AI Has %d pieces."), PlayerPiecesCanMove.Num()));

						// If there are player's pieces eligible to move
						if (PlayerPiecesCanMove.Num() > 0)
						{
							// Select randomly the index of the piece to move
							int8 RandPawnIdx = FMath::Rand() % PlayerPiecesCanMove.Num();
							int8 RandPieceNum = PlayerPiecesCanMove[RandPawnIdx].first;
							TArray<std::pair<int8, int8>> AttackableTiles = GameMode->TurnPossibleMoves[RandPieceNum];
							// Select randomly the index of the tile to move to
							int8 RandNewTile = FMath::Rand() % AttackableTiles.Num();

							int8 OldX = GameMode->GField->PawnArray[RandPieceNum]->GetGridPosition()[0];
							int8 OldY = GameMode->GField->PawnArray[RandPieceNum]->GetGridPosition()[1];
							int8 NewX = AttackableTiles[RandNewTile].first;
							int8 NewY = AttackableTiles[RandNewTile].second;

							if (GameMode->GField->IsValidTile(OldX, OldY)
								&& GameMode->GField->IsValidTile(NewX, NewY))
							{
								// Make the selected move
								bool EatFlag = GameMode->MakeMove(GameMode->GField->PawnArray[RandPieceNum], NewX, NewY);

								// Pawn promotion handling
								int8 OpponentSide = Color == EPawnColor::WHITE ? GameMode->GField->Size - 1 : 0;
								if (NewX == OpponentSide && GameMode->GField->PawnArray[RandPieceNum]->GetType() == EPawnType::PAWN)
								{
									// Randomly choice of what to promote to
									int8 RandSpawnPawn = FMath::Rand() % 4;
									switch (RandSpawnPawn)
									{
									case 0: GameMode->SetPawnPromotionChoice(EPawnType::QUEEN); break;
									case 1: GameMode->SetPawnPromotionChoice(EPawnType::ROOK);  break;
									case 2: GameMode->SetPawnPromotionChoice(EPawnType::BISHOP); break;
									case 3: GameMode->SetPawnPromotionChoice(EPawnType::KNIGHT); break;
									}
								}
								else
								{
									// End Turn
									GameMode->LastPiece = GameMode->GField->PawnArray[RandPieceNum];
									GameMode->LastEatFlag = EatFlag;
									GameMode->EndTurn(PlayerNumber);
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

void AChess_RandomPlayer::OnWin()
{
	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode && GameInstance)
	{
		FString Msg = UChess_GameInstance::RANDOM_WIN_1;
		if (GameMode->Players.Num() == 3) // AI vs AI
			Msg += FString::FromInt(PlayerNumber + 1) + " ";

		Msg += UChess_GameInstance::RANDOM_WIN_2;
		GameInstance->SetTurnMessage(Msg);

		PlayerNumber ?
			GameInstance->IncrementScorePlayer_2() :
			GameInstance->IncrementScorePlayer_1();
	}
}

void AChess_RandomPlayer::OnLose()
{
	// GameInstance->SetTurnMessage(UChess_GameInstance::RANDOM_DEFEAT);
}