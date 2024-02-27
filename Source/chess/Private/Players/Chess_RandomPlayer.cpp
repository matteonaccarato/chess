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
	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("AI (Random) Turn"));
	GameInstance->SetTurnMessage(TEXT("AI (Random) Turn"));

	FTimerHandle TimerHandle;
	// e.g. RandTimer = 23 => Means a timer of 2.3 seconds
	// RandTimer [1.0, 3.0] seconds
	// TODO: sono magic numberss
	int8 RandTimer = FMath::Rand() % 21 + 10;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
		{
			// TODO: fare in modo meno oneroso (tipo che ogni giocatore ha come attributo una mappa
			// { pedina: posizione }

			AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
			if (GameMode != nullptr)
			{
				// <PAWN, <DELTAX, DELTAY>>
				// TArray<FPawnsPossibilites> MyPawns;
				TArray<ABasePawn*> MyPawns; // pawns che si possono muovere 

				for (auto& CurrPawn : GameMode->GField->GetPawnArray())
				{
					// TODO: third condition is test only
					// TODO => eat possibility
					TArray<FSteps> PossibleSteps;
					if (CurrPawn->GetColor() == EPawnColor::BLACK && CurrPawn->GetStatus() == EPawnStatus::ALIVE)
					{

						// il primo tarray corrisponde al pawn che può effettuare la mossa, 
						// il secondo tarray indica le tiles attaccabili
						TArray<std::pair<int8, int8>> Tmp = GameMode->ShowPossibleMoves(CurrPawn, true);
						if (Tmp.Num() > 0)
						{
							AttackableTiles.Add(Tmp);
							MyPawns.Add(CurrPawn);
						}
						
					}
				}

				GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("AI Has %d pawns."), MyPawns.Num()));



				if (MyPawns.Num() > 0)
				{
					int8 RandPawnIdx = FMath::Rand() % MyPawns.Num(); // select pawn to move
					int8 RandNewTile = FMath::Rand() % AttackableTiles[RandPawnIdx].Num();
					int32 OldX = MyPawns[RandPawnIdx]->GetGridPosition()[0];
					int32 OldY = MyPawns[RandPawnIdx]->GetGridPosition()[1];
					int8 NewX = AttackableTiles[RandPawnIdx][RandNewTile].first;
					int8 NewY = AttackableTiles[RandPawnIdx][RandNewTile].second;
					TArray<ATile*> TilesArray = GameMode->GField->GetTileArray();

					bool EatFlag = static_cast<int>(TilesArray[NewX * GameMode->GField->Size + NewY]->GetTileStatus().PawnColor) == -static_cast<int>(MyPawns[RandPawnIdx]->GetColor());
					if (EatFlag)
					{
						// TODO => c'è qualcosa da fare come destroy / deallocazione ?
						ABasePawn* PawnToEat = TilesArray[NewX * GameMode->GField->Size + NewY]->GetPawn();

						PawnToEat->SetStatus(EPawnStatus::DEAD);
						GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Yellow, FString::Printf(TEXT("%f %f pawn has been eaten"), PawnToEat->GetGridPosition()[0], PawnToEat->GetGridPosition()[1]));
						// Hides visible components
						PawnToEat->SetActorHiddenInGame(true);

						// Disables collision components
						PawnToEat->SetActorEnableCollision(false);

						// Stops the Actor from ticking
						PawnToEat->SetActorTickEnabled(false);
					}



					TilesArray[OldX * GameMode->GField->Size + OldY]->SetPlayerOwner(PlayerNumber);
					TilesArray[OldX * GameMode->GField->Size + OldY]->SetTileStatus({ 1, EPawnColor::NONE, EPawnColor::NONE, EPawnType::NONE });
					TilesArray[OldX * GameMode->GField->Size + OldY]->SetPawn(nullptr);
					// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("SET %d %d TO EMPTY"), OldX, OldY));


					TilesArray[NewX * GameMode->GField->Size + NewY]->SetPlayerOwner(PlayerNumber);
					TilesArray[NewX * GameMode->GField->Size + NewY]->SetTileStatus({ 0, EPawnColor::NONE, MyPawns[RandPawnIdx]->GetColor(), MyPawns[RandPawnIdx]->GetType() });
					TilesArray[NewX * GameMode->GField->Size + NewY]->SetPawn(MyPawns[RandPawnIdx]);
					// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("SET %d %d TO PAWN"), NewX, NewY));

					FVector SpawnPosition = GameMode->GField->GetRelativeLocationByXYPosition(NewX, NewY) + FVector(0, 0, MyPawns[RandPawnIdx]->GetActorLocation()[2]);
					MyPawns[RandPawnIdx]->SetActorLocation(SpawnPosition);
					MyPawns[RandPawnIdx]->SetGridPosition(NewX, NewY);

				
					GameMode->SetCellPawn(PlayerNumber, SpawnPosition);

				}
				else
				{
					GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("AI cannot move anything"));
					GameMode->SetCellPawn(-1, FVector());
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
	/// TODO
	// GameInstance->SetTurnMessage(TEXT("AI Loses"));
}

