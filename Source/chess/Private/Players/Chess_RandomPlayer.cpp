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
			if (GameMode && GameMode->CanPlay)
			{
				// Setting all tiles as NON attackable from Nobody
				for (auto& Tile : GameMode->GField->GetTileArray())
				{
					FTileStatus TileStatus = Tile->GetTileStatus();
					TileStatus.AttackableFrom.SetNum(2, false);
					TileStatus.WhoCanGo.Empty();
					Tile->SetTileStatus(TileStatus);
				}


				// TArray which will contain black pawns which can make moves
				TArray<ABasePawn*> MyPawns; 
				for (auto& CurrPawn : GameMode->GField->GetPawnArray())
				{
					if (CurrPawn->GetColor() == EPawnColor::BLACK && CurrPawn->GetStatus() == EPawnStatus::ALIVE)
					{
						/* Compute eligible moves for CurrPawn */

						// Backup current checkflag
						EPawnColor TmpCheckFlag = GameMode->CheckFlag;

						// Obtain as TArray the possible tiles where CurrPawn can move itself
						TArray<std::pair<int8, int8>> Tmp = GameMode->ShowPossibleMoves(CurrPawn, true, false, true);

						// Restore previous checkFlag
						GameMode->CheckFlag = TmpCheckFlag;

						// If CurrPawn can make some eligible moves, it is pushed to MyPawns
						// and the tiles it can attack to AttackableTiles
						if (Tmp.Num() > 0)
						{
							AttackableTiles.Add(Tmp);
							MyPawns.Add(CurrPawn);
						}
					}
				}
				GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("AI Has %d pawns."), MyPawns.Num()));

				// If there are black pawns eligible to move
				if (MyPawns.Num() > 0)
				{
					// Select randomically the index of the pawn to move
					int8 RandPawnIdx = FMath::Rand() % MyPawns.Num(); 
					// Select randomically the index of the tile to move to
					int8 RandNewTile = FMath::Rand() % AttackableTiles[RandPawnIdx].Num();

					int32 OldX = MyPawns[RandPawnIdx]->GetGridPosition()[0];
					int32 OldY = MyPawns[RandPawnIdx]->GetGridPosition()[1];
					int8 NewX = AttackableTiles[RandPawnIdx][RandNewTile].first;
					int8 NewY = AttackableTiles[RandPawnIdx][RandNewTile].second;
					TArray<ATile*> TilesArray = GameMode->GField->GetTileArray();

					// EatFlag is true if the Tile->PawnColor is the opposite of the black pawn
					// e.g. Tile->PawnwColor = 1 (white) , Pawn->Color = -1 => EatFlag = true
					// e.g. Tile->PawnwColor = 0 (empty) , Pawn->Color = -1 => EatFlag = flag
					bool EatFlag = static_cast<int>(TilesArray[NewX * GameMode->GField->Size + NewY]->GetTileStatus().PawnColor) == -static_cast<int>(MyPawns[RandPawnIdx]->GetColor());
					if (EatFlag)
					{
						ABasePawn* PawnToEat = TilesArray[NewX * GameMode->GField->Size + NewY]->GetPawn();
						if (PawnToEat)
							GameMode->GField->DespawnPawn(PawnToEat->GetGridPosition()[0], PawnToEat->GetGridPosition()[1]);
					}

					// TilesArray[OldX * GameMode->GField->Size + OldY]->ClearInfo();

					// Clear starting tile (no player owner, no pawn on it, ...)
					// Update ending tile (new player owner, new tile status, new pawn)
					MyPawns[RandPawnIdx]->Move(TilesArray[OldX * GameMode->GField->Size + OldY], TilesArray[NewX * GameMode->GField->Size + NewY]);

					// TODO => superfluo (?, gi� fatto in gamemode)
					if (MyPawns[RandPawnIdx]->GetType() == EPawnType::PAWN)
					{
						MyPawns[RandPawnIdx]->SetMaxNumberSteps(1);
					}

					// Update last move (useful when doing pawn promotion)
					GameMode->LastGridPosition = FVector2D(NewX, NewY);
					GameMode->PreviousGridPosition = FVector2D(OldX, OldY);
					

					// Pawn promotion handling
					if (NewX == 0 && MyPawns[RandPawnIdx]->GetType() == EPawnType::PAWN)
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

					// End Turn
					GameMode->IsCheck();
					GameMode->AddToReplay(MyPawns[RandPawnIdx], EatFlag);
					GameMode->EndTurn(PlayerNumber);
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

