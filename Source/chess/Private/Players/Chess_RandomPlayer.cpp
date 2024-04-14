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
	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("AI (Random)"));
	if (GameInstance)
		GameInstance->SetTurnMessage(TEXT("AI (Random)"));

	if (GameMode)
	{
		FTimerHandle TimerHandle;
		// e.g. RandTimer = 23 => Means a timer of 2.3 seconds
		// RandTimer [1.0, 3.0] seconds
		// TODO: sono magic numberss, fare file .ini (o .json) per valori di configurazione (li legge una classe padre, valori statici)
		int8 RandTimer = GameMode->bIsHumanPlaying ? FMath::Rand() % 21 + 10 : 1;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
			{
				if (IsMyTurn)
				{
					AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
					if (GameMode && GameMode->ReplayInProgress == 0)
					{
						// Setting all tiles as NON attackable from Nobody ( TODO => già fatto nell'inizio turno)
						for (auto& Tile : GameMode->GField->GetTileArray())
						{
							FTileStatus TileStatus = Tile->GetTileStatus();
							TileStatus.AttackableFrom[0] = 0; TileStatus.AttackableFrom[1] = 0;
							TileStatus.WhoCanGo.Empty();
							Tile->SetTileStatus(TileStatus);
						}



						TArray<int8>& PlayerPiecesCanMove = Color == EPawnColor::WHITE ? GameMode->WhitePiecesCanMove : GameMode->BlackPiecesCanMove;





						GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("AI Has %d pawns."), PlayerPiecesCanMove.Num()));

						// If there are black pawns eligible to move
						if (PlayerPiecesCanMove.Num() > 0)
						{
							// Select randomically the index of the pawn to move
							int8 RandPawnIdx = FMath::Rand() % PlayerPiecesCanMove.Num();
							int8 RandPieceNum = PlayerPiecesCanMove[RandPawnIdx];
							TArray<std::pair<int8, int8>> AttackableTiles = GameMode->TurnPossibleMoves[RandPieceNum];
							// Select randomically the index of the tile to move to
							int8 RandNewTile = FMath::Rand() % AttackableTiles.Num();

							int32 OldX = GameMode->GField->PawnArray[RandPieceNum]->GetGridPosition()[0];
							int32 OldY = GameMode->GField->PawnArray[RandPieceNum]->GetGridPosition()[1];
							int8 NewX = AttackableTiles[RandNewTile].first;
							int8 NewY = AttackableTiles[RandNewTile].second;

							if (GameMode->GField->IsValidTile(OldX, OldY)
								&& GameMode->GField->IsValidTile(NewX, NewY))
							{

								bool EatFlag = GameMode->MakeMove(GameMode->GField->PawnArray[RandPieceNum], NewX, NewY);


								// Pawn promotion handling
								int8 OpponentSide = Color == EPawnColor::WHITE ? GameMode->GField->Size - 1 : 0;
								if (NewX == OpponentSide && GameMode->GField->PawnArray[RandPieceNum]->GetType() == EPawnType::PAWN)
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
									GameMode->LastPiece = GameMode->GField->PawnArray[RandPieceNum];
									GameMode->LastEatFlag = EatFlag;
									GameMode->EndTurn(PlayerNumber);
								}
							}
						}
						else
						{
							// TODO => rimuovere
							// No pieces can make eligible moves => BLACK is checkmated
							GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Yellow, TEXT("BRO | smth strange happened. u should not be here!"));

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

void AChess_RandomPlayer::OnWin()
{
	// TODO
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

void AChess_RandomPlayer::OnLose()
{
	// TODO
	// GameInstance->SetTurnMessage(TEXT("AI Loses"));
}