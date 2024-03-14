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
	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("AI (Random)"));
	GameInstance->SetTurnMessage(TEXT("AI (Random)"));

	FTimerHandle TimerHandle;
	// e.g. RandTimer = 23 => Means a timer of 2.3 seconds
	// RandTimer [1.0, 3.0] seconds
	// TODO: sono magic numberss, fare file .ini (o .json) per valori di configurazione (li legge una classe padre, valori statici)
	int8 RandTimer = FMath::Rand() % 21 + 10;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
		{
			AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
			if (GameMode && GameMode->ReplayInProgress == 0 && IsMyTurn)
			{
				// Setting all tiles as NON attackable from Nobody (TODO => già fatto nell'inizio turno)
				for (auto& Tile : GameMode->GField->GetTileArray())
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
					// Select randomically the index of the pawn to move
					int8 RandPawnIdx = FMath::Rand() % GameMode->BlackPiecesCanMove.Num();
					int8 RandPieceNum = GameMode->BlackPiecesCanMove[RandPawnIdx];
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
						TArray<ATile*> TilesArray = GameMode->GField->GetTileArray();

						// EatFlag is true if the Tile->PawnColor is the opposite of the black pawn
						// e.g. Tile->PawnwColor = 1 (white) , Pawn->Color = -1 => EatFlag = true
						// e.g. Tile->PawnwColor = 0 (empty) , Pawn->Color = -1 => EatFlag = flag
						bool EatFlag = static_cast<int>(TilesArray[NewX * GameMode->GField->Size + NewY]->GetTileStatus().PawnColor) == -static_cast<int>(GameMode->GField->PawnArray[RandPieceNum]->GetColor());
						if (EatFlag)
						{
							ABasePawn* PawnToEat = TilesArray[NewX * GameMode->GField->Size + NewY]->GetPawn();
							if (PawnToEat)
								GameMode->GField->DespawnPawn(PawnToEat->GetGridPosition()[0], PawnToEat->GetGridPosition()[1]);
						}

						// TilesArray[OldX * GameMode->GField->Size + OldY]->ClearInfo();

						// Clear starting tile (no player owner, no pawn on it, ...)
						// Update ending tile (new player owner, new tile status, new pawn)
						GameMode->GField->PawnArray[RandPieceNum]->Move(TilesArray[OldX * GameMode->GField->Size + OldY], TilesArray[NewX * GameMode->GField->Size + NewY]);


						// Castling Handling (King moves by two tiles)
						if (GameMode->GField->PawnArray[RandPieceNum]->GetType() == EPawnType::KING
							&& FMath::Abs(NewY - OldY) == 2)
						{
							// Move the rook
							bool ShortCastling = (NewY - OldY) > 0;
							int8 RookX = GameMode->GField->PawnArray[RandPieceNum]->GetColor() == EPawnColor::WHITE ? 0 : 7;
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

						if (GameMode->GField->PawnArray[RandPieceNum]->GetType() == EPawnType::ROOK)
						{
							GameMode->CastlingInfoBlack.RooksMoved[NewY == 0 ? 0 : 1] = true;
						}



						// TODO => superfluo (?, già fatto in gamemode)
						if (GameMode->GField->PawnArray[RandPieceNum]->GetType() == EPawnType::PAWN)
						{
							GameMode->GField->PawnArray[RandPieceNum]->SetMaxNumberSteps(1);
						}

						// Update last move (useful when doing pawn promotion)
						GameMode->LastGridPosition = FVector2D(NewX, NewY);
						GameMode->PreviousGridPosition = FVector2D(OldX, OldY);
						GameMode->LastEatFlag = EatFlag;
					

						// Pawn promotion handling
						if (NewX == 0 && GameMode->GField->PawnArray[RandPieceNum]->GetType() == EPawnType::PAWN)
						{
							// Randomically choice of what to promote to
							int8 RandSpawnPawn = FMath::Rand() % 4;
							switch (RandPawnIdx)
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
							GameMode->IsCheck();
							GameMode->AddToReplay(GameMode->GField->PawnArray[RandPieceNum], EatFlag);
							GameMode->EndTurn(PlayerNumber);
						}

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
		}, RandTimer/10.f, false);
}

void AChess_RandomPlayer::OnWin()
{
	// TODO
	GameInstance->SetTurnMessage(TEXT("AI Wins"));
	GameInstance->IncrementScoreAiPlayer();
}

void AChess_RandomPlayer::OnLose()
{
	// TODO
	// GameInstance->SetTurnMessage(TEXT("AI Loses"));
}

