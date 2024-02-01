// Fill out your copyright notice in the Description page of Project Settings.

#include "Chess_GameMode.h"
#include "GameField.h"
#include "Players/Chess_PlayerController.h"
#include "Players/Chess_HumanPlayer.h"
#include "Players/Chess_RandomPlayer.h"
#include "Players/Chess_MiniMaxPlayer.h"
#include "EngineUtils.h"

AChess_GameMode::AChess_GameMode()
{
	PlayerControllerClass = AChess_PlayerController::StaticClass();
	DefaultPawnClass = AChess_HumanPlayer::StaticClass();
	FieldSize = 8;
}

void AChess_GameMode::BeginPlay()
{
	Super::BeginPlay();

	IsGameOver = false;
	MoveCounter = 0;

	AChess_HumanPlayer* HumanPlayer = Cast<AChess_HumanPlayer>(*TActorIterator<AChess_HumanPlayer>(GetWorld()));

	if (GameFieldClass != nullptr)
	{
		GField = GetWorld()->SpawnActor<AGameField>(GameFieldClass);
		GField->Size = FieldSize;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameFieldClass is null"));
	}

	float CameraPosX = ((GField->TileSize * (FieldSize + ((FieldSize - 1) * GField->NormalizedCellPadding) - (FieldSize - 1))) / 2) - (GField->TileSize / 2);
	FVector CameraPos(CameraPosX, CameraPosX, 1000.0f); // TODO: 1000 da mettere come attributo
	HumanPlayer->SetActorLocationAndRotation(CameraPos, FRotationMatrix::MakeFromX(FVector(0, 0, -1)).Rotator());
	
	// Human player = 0
	Players.Add(HumanPlayer);
	// Random player
	auto* AI = GetWorld()->SpawnActor<AChess_RandomPlayer>(FVector(), FRotator());
	// Minimax player
	// auto* AI = GetWorld()->SpawnActor<AChess_MiniMaxPlayer>(FVector(), FRotator());

	Players.Add(AI);


	this->ChoosePlayerAndStartGame();
}

void AChess_GameMode::ChoosePlayerAndStartGame()
{
	CurrentPlayer = 0;
	for (int32 i = 0; i < Players.Num(); i++)
	{
		Players[i]->PlayerNumber = i;
		Players[i]->Color = i == CurrentPlayer ? EColor::W : EColor::B;
	}

	MoveCounter += 1;

	Players[CurrentPlayer]->OnTurn();
}

void AChess_GameMode::SetCellPawn(const int32 PlayerNumber, const FVector& SpawnPosition)
{
	

	// -1 to notify draw
	if (PlayerNumber == -1)
	{
		IsGameOver = true;
		GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("DRAW"));

		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
			{
				GField->ResetField();
			}, 3, false);
	}
	else
	{
		if (IsGameOver || PlayerNumber != CurrentPlayer)
			return;

		TurnNextPlayer();
	}

	/*
	
	TODO
	
	*/

	// FVector Location = GField->GetActorLocation() + SpawnPosition + FVector(0, 0, 10);
	// GetWorld()->SpawnActor(B_KingClass, &Location);
	// ABasePawn* BasePawnObj = GetWorld()->SpawnActor<ABasePawn>(B_KingClass, Location, FRotator(0,90,0));
	/*if (BasePawnObj != nullptr)
	{
		// FVector2D xy = GField->GetXYPositionByRelativeLocation(Location);
		// BasePawnObj->SetGridPosition(xy[0], xy[1]);
		const float TileScale = GField->TileSize / 100; // TileScale as attribute
		BasePawnObj->SetActorScale3D(FVector(TileScale * 0.8, TileScale * 0.8, 0.05));
	}*/


	// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("Set"));


	// UClass* ColorActor = Players[CurrentPlayer]->Color == E

	
}

int32 AChess_GameMode::GetNextPlayer(int32 Player)
{
	Player++;
	if (!Players.IsValidIndex(Player))
	{
		Player = 0;
	}
	return Player;
}

void AChess_GameMode::TurnNextPlayer()
{
	MoveCounter += 1;
	CurrentPlayer = GetNextPlayer(CurrentPlayer);
	// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("MOVE COUNTER %d"), MoveCounter));
	Players[CurrentPlayer]->OnTurn();
}

bool AChess_GameMode::IsValidMove(ABasePawn* Pawn/*, const ATile* CurrTile*/, const int8 NewX, const int8 NewY)
{
	bool IsValid = false;

	if (NewX >= 0 && NewX < GField->Size
		&& NewY >= 0 && NewY < GField->Size)
	{
		TArray<ATile*> TileArray = GField->GetTileArray();
		ATile* NewTile = TileArray[NewX * GField->Size + NewY];

	

		// CurrTile => remove it

		FVector2D NewGridPosition = NewTile->GetGridPosition();
		FVector2D CurrGridPosition = Pawn->GetGridPosition();
	


		// NewGridPosition , CurrPawnPosition
		EPawnColor DirectionFlag = Pawn->GetColor();
		int8 DeltaX = (NewGridPosition[0] - CurrGridPosition[0]) * static_cast<double>(DirectionFlag);
		int8 DeltaY = NewGridPosition[1] - CurrGridPosition[1];



		if (NewTile->GetTileStatus().EmptyFlag)
		{
			switch (Pawn->GetMovement())
			{
				// TODO: can be a mix of them
				// array of possible movements
			case EPawnMovement::FORWARD:


				if (DeltaY == 0 && DeltaX >= 0 && DeltaX <= Pawn->GetMaxNumberSteps())
				{
					// TODO => unione solo tipo senza colore
					if (Pawn->GetType() == EPawnType::PAWN)
					{
						Pawn->SetMaxNumberSteps(1);
					}
					IsValid = true;
				}

				break;
				/*case EPawnMovement::BACKWARD: break;
				case EPawnMovement::LEFT: break;
				case EPawnMovement::RIGHT: break;
				case EPawnMovement::DIAGONAL: break;*/

			}
		}
	}


	return IsValid;
}
