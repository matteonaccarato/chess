// Fill out your copyright notice in the Description page of Project Settings.


#include "Players/Chess_RandomPlayer.h"

// Sets default values
AChess_RandomPlayer::AChess_RandomPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GameInstace = Cast<UChess_GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
}

// Called when the game starts or when spawned
void AChess_RandomPlayer::BeginPlay()
{
	Super::BeginPlay();	
}

// Called every frame
void AChess_RandomPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AChess_RandomPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AChess_RandomPlayer::OnTurn()
{
	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("AI (Random) Turn"));

	FTimerHandle TimerHandle;
	// e.g. RandTimer = 23 => Means a timer of 2.3 seconds
	// RandTimer [1.0, 3.0] seconds
	// int8 RandTimer = FMath::Rand() % 20 + 10;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
		{


			// TODO: fare in modo meno oneroso (tipo che ogni giocatore ha come attributo una mappa
	// pedina: posizione

			TArray<ABasePawn*> MyPawns;

			AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
			if (GameMode != nullptr)
			{
				for (auto& CurrPawn : GameMode->GField->GetPawnArray())
				{
					// TODO: second condition is test only
					if (CurrPawn->GetColor() == EPawnsColors::BLACK && CurrPawn->GetType() == ETileStatus::B_PAWN)
					{
						MyPawns.Add(CurrPawn);
					}
				}

				// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("AI Has %d pawns."), MyPawns.Num()));


				// choose random pawn in mypawns
				// move it in function of its params (movement and max steps)
				// do action since it results in valid move

				// notify if action has been taken (-1)
				// or it ran out of possibiliities (all pawns are blocked)
				// int8 AttemptsCounter = 0;
				bool MoveMade = false;
				while (!MoveMade && MyPawns.Num() > 0)
				{

					TArray<ATile*> Tiles = GameMode->GField->GetTileArray();

					int32 RandIdx = FMath::Rand() % MyPawns.Num();
					// PawnAttempt[RandIdx] = 1;


					// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("AI got %d."), RandIdx));



					int32 OldX = MyPawns[RandIdx]->GetGridPosition()[0];
					int32 OldY = MyPawns[RandIdx]->GetGridPosition()[1];
					int32 NewX = OldX;
					int32 NewY = OldY;

					// FVector Location = GameMode->GField->GetRelativeLocationByXYPosition((MyPawns[RandIdx])->GetGridPosition()[0], (MyPawns[RandIdx])->GetGridPosition()[1]);


					switch (MyPawns[RandIdx]->GetType())
					{
					case ETileStatus::B_PAWN:
						int32 RandSteps = (FMath::Rand() % MyPawns[RandIdx]->GetMaxNumberSteps()) + 1;
						NewX -= RandSteps;
						break;
					}


					if (GameMode->IsValidMove(MyPawns[RandIdx], NewX, NewY))
					{


						Tiles[OldX * 8 + OldY]->SetTileStatus(PlayerNumber, ETileStatus::EMPTY);
						// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("SET %d %d TO EMPTY"), OldX, OldY));


						Tiles[NewX * 8 + NewY]->SetTileStatus(PlayerNumber, MyPawns[RandIdx]->GetType());
						// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("SET %d %d TO PAWN"), NewX, NewY));

						FVector SpawnPosition = GameMode->GField->GetRelativeLocationByXYPosition(NewX, NewY) + FVector(0, 0, MyPawns[RandIdx]->GetActorLocation()[2]);
						MyPawns[RandIdx]->SetActorLocation(SpawnPosition);
						MyPawns[RandIdx]->SetGridPosition(NewX, NewY);

						MoveMade = true;
						GameMode->SetCellPawn(PlayerNumber, SpawnPosition);



					}
					else
					{
						// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("Invalid move"));
						// AttemptsCounter += 1;
						MyPawns.RemoveAt(RandIdx);
					}

					// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("MyPawns Num %d"), MyPawns.Num()));

				}

				if (MyPawns.Num() < 1)
				{
					GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("AI cannot move anything"));
					GameMode->SetCellPawn(-1, FVector());
				}


			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("GameMode is null"));
			}







		}, 3, false);

	
	
	


	// GameMode->SetCellPawn(PlayerNumber, FVector());
}

void AChess_RandomPlayer::OnWin()
{
	// TODO
}

void AChess_RandomPlayer::OnLose()
{
	/// TODO
}

