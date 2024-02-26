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
				TArray<FPawnsPossibilites> MyPawns;
				for (auto& CurrPawn : GameMode->GField->GetPawnArray())
				{
					// TODO: third condition is test only
					// TODO => eat possibility
					TArray<FSteps> PossibleSteps;
					if (CurrPawn->GetColor() == EPawnColor::BLACK && CurrPawn->GetStatus() == EPawnStatus::ALIVE /*  && CurrPawn->GetType() == EPawnType::PAWN */)
					{
						for (const auto& direction : CurrPawn->GetCardinalDirections())
						{
							// ECardinalDirection CurrDirection = *it;
							FSteps Steps{};

							for (int i = 0; i < CurrPawn->GetMaxNumberSteps(); i++)
							{
								Steps.CardinalDirection = direction;
								Steps.Number = i + 1;
								PossibleSteps.Add(Steps);
							}
						}
						
						// TODO descrivere funzionamento
						FPawnsPossibilites PawnPossibilites;
						PawnPossibilites.Pawn = CurrPawn;
						PawnPossibilites.PossibleSteps = PossibleSteps;
						MyPawns.Add(PawnPossibilites);
					}
				}
				GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("AI Has %d pawns."), MyPawns.Num()));


				// choose random pawn in mypawns
				// move it in function of its params (movement and max steps)
				// do action since it results in valid move
				bool MoveMade = false;
				while (!MoveMade && MyPawns.Num() > 0)
				{
					TArray<ATile*> Tiles = GameMode->GField->GetTileArray();
					int32 RandIdx = FMath::Rand() % MyPawns.Num();

					// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("AI got %d."), RandIdx));

					int32 OldX = MyPawns[RandIdx].Pawn->GetGridPosition()[0];
					int32 OldY = MyPawns[RandIdx].Pawn->GetGridPosition()[1];
			
					int8 RandStepIdx = FMath::Rand() % MyPawns[RandIdx].PossibleSteps.Num();
					int8 RandStepsNumber = MyPawns[RandIdx].PossibleSteps[RandStepIdx].Number;
					ECardinalDirection RandStepDirection = MyPawns[RandIdx].PossibleSteps[RandStepIdx].CardinalDirection;
					int8 XOffset = 0, YOffset = 0;
					int8 FlagDirection = 0;

					MyPawns[RandIdx].PossibleSteps.RemoveAt(RandStepIdx); // delete this movement possibility

					switch(RandStepDirection)
					{
					case ECardinalDirection::NORTH:
						FlagDirection = 1;
					case ECardinalDirection::SOUTH:
						// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("FlagDirection %d"), FlagDirection));
						FlagDirection = FlagDirection ? FlagDirection : -1;
						XOffset = RandStepsNumber * FlagDirection;
						YOffset = 0;
						break;

					case ECardinalDirection::NORTHEAST:
						FlagDirection = 1;
					case ECardinalDirection::SOUTHWEST:
						FlagDirection = FlagDirection ? FlagDirection : -1;
						XOffset = RandStepsNumber * FlagDirection;
						YOffset = RandStepsNumber * FlagDirection;
						break;

					case ECardinalDirection::EAST:
						FlagDirection = 1;
					case ECardinalDirection::WEST:
						FlagDirection = FlagDirection ? FlagDirection : -1;
						XOffset = 0;
						YOffset = RandStepsNumber * FlagDirection;
						break;

					case ECardinalDirection::NORTHWEST:
						FlagDirection = 1;
					case ECardinalDirection::SOUTHEAST:
						FlagDirection = FlagDirection ? FlagDirection : -1;
						XOffset = RandStepsNumber * FlagDirection;
						YOffset = RandStepsNumber * (-FlagDirection);
						break;

					case ECardinalDirection::KNIGHT_TL:
						YOffset = -1;
					case ECardinalDirection::KNIGHT_TR:
						XOffset = 2;
						YOffset = YOffset ? YOffset : 1;
						break;

					case ECardinalDirection::KNIGHT_RT:
						XOffset = 1;
					case ECardinalDirection::KNIGHT_RB:
						XOffset = XOffset ? XOffset : -1;
						YOffset = 2;
						break;

					case ECardinalDirection::KNIGHT_BR:
						YOffset = 1;
					case ECardinalDirection::KNIGHT_BL:
						XOffset = -2;
						YOffset = YOffset ? YOffset : -1;
						break;

					case ECardinalDirection::KNIGHT_LT:
						XOffset = 1;
					case ECardinalDirection::KNIGHT_LB:
						XOffset = XOffset ? XOffset : -1;
						YOffset = -2;
						break;
						
					}


					// TODO => vedere se ho fatto solo su X o Y la moltiplicazione per il colore
					XOffset = XOffset * static_cast<int>(MyPawns[RandIdx].Pawn->GetColor());
					// YOffset = YOffset * static_cast<int>(MyPawns[RandIdx].Pawn->GetColor());


					int8 NewX = OldX + XOffset;
					int8 NewY = OldY + YOffset;


					if (GameMode->IsValidMove(MyPawns[RandIdx].Pawn, NewX, NewY /*, EatFlag*/))
					{
						bool EatFlag = static_cast<int>(GameMode->GField->GetTileArray()[NewX * GameMode->GField->Size + NewY]->GetTileStatus().PawnColor) == -static_cast<int>(MyPawns[RandIdx].Pawn->GetColor());
						if (EatFlag)
						{
							// TODO => c'è qualcosa da fare come destroy / deallocazione ?
							ABasePawn* PawnToEat = GameMode->GField->GetTileArray()[NewX * GameMode->GField->Size + NewY]->GetPawn();

							PawnToEat->SetStatus(EPawnStatus::DEAD);
							GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Blue, FString::Printf(TEXT("%f %f pawn has been eaten"), PawnToEat->GetGridPosition()[0], PawnToEat->GetGridPosition()[1]));
							// Hides visible components
							PawnToEat->SetActorHiddenInGame(true);

							// Disables collision components
							PawnToEat->SetActorEnableCollision(false);

							// Stops the Actor from ticking
							PawnToEat->SetActorTickEnabled(false);
						}



						Tiles[OldX * GameMode->GField->Size + OldY]->SetPlayerOwner(PlayerNumber);
						Tiles[OldX * GameMode->GField->Size + OldY]->SetTileStatus({ 1, EPawnColor::NONE, EPawnColor::NONE, EPawnType::NONE });
						Tiles[OldX * GameMode->GField->Size + OldY]->SetPawn(nullptr);
						// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("SET %d %d TO EMPTY"), OldX, OldY));


						Tiles[NewX * GameMode->GField->Size + NewY]->SetPlayerOwner(PlayerNumber);
						Tiles[NewX * GameMode->GField->Size + NewY]->SetTileStatus({ 0, EPawnColor::NONE, MyPawns[RandIdx].Pawn->GetColor(), MyPawns[RandIdx].Pawn->GetType() });
						Tiles[NewX * GameMode->GField->Size + NewY]->SetPawn(MyPawns[RandIdx].Pawn);
						// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("SET %d %d TO PAWN"), NewX, NewY));

						FVector SpawnPosition = GameMode->GField->GetRelativeLocationByXYPosition(NewX, NewY) + FVector(0, 0, MyPawns[RandIdx].Pawn->GetActorLocation()[2]);
						MyPawns[RandIdx].Pawn->SetActorLocation(SpawnPosition);
						MyPawns[RandIdx].Pawn->SetGridPosition(NewX, NewY);

						MoveMade = true;
						GameMode->SetCellPawn(PlayerNumber, SpawnPosition);

					}
					else
					{
						
						// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("Invalid move")));
						if (MyPawns[RandIdx].PossibleSteps.Num() == 0)
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

