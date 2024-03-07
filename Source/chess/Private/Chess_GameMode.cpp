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
	PawnPromotionType = EPawnType::NONE;
	LastGridPosition = FVector2D(-1, -1);
}

void AChess_GameMode::BeginPlay()
{
	Super::BeginPlay();

	IsGameOver = false;
	CanPlay = true;
	MoveCounter = 0;
	CheckFlag = EPawnColor::NONE;
	CheckMateFlag = EPawnColor::NONE;

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
	FVector CameraPos(CameraPosX, CameraPosX, 1250.0f); // TODO: 1000 da mettere come attributo
	HumanPlayer->SetActorLocationAndRotation(CameraPos, FRotationMatrix::MakeFromX(FVector(0, 0, -1)).Rotator());
	
	// Human player = 0
	Players.Add(HumanPlayer);
	// Random player
	auto* AI = GetWorld()->SpawnActor<AChess_RandomPlayer>(FVector(), FRotator());
	// Minimax player
	// auto* AI = GetWorld()->SpawnActor<AChess_MiniMaxPlayer>(FVector(), FRotator());

	Players.Add(AI);


	



	UWorld* World = GetWorld();
	if (World && ReplayWidgetRef)
	{
		ReplayWidget = CreateWidget<UUserWidget>(World, ReplayWidgetRef, FName("Replay"));
		ReplayWidget->AddToViewport(0);
		GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("aa")));
	}





	this->ChoosePlayerAndStartGame();
}

void AChess_GameMode::ChoosePlayerAndStartGame()
{
	CurrentPlayer = 0;
	for (int32 i = 0; i < Players.Num(); i++)
	{
		Players[i]->PlayerNumber = i;
		Players[i]->Color = i == CurrentPlayer ? EPawnColor::WHITE : EPawnColor::BLACK;
	}

	MoveCounter += 1;

	Players[CurrentPlayer]->OnTurn();
}

void AChess_GameMode::EndTurn(const int32 PlayerNumber)
{
	// MoveCounter += 1;

	// -1 to notify checkmate
	if (PlayerNumber == -1)
	{
		IsGameOver = true;
		GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("%d under CHECK MATE"), CheckMateFlag));

		Players[CurrentPlayer]->OnWin();
		for (int32 i = 0; i < Players.Num(); i++)
			if (i != CurrentPlayer)
				Players[i]->OnLose();
			
		

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



		/* if (IsCheckMate())
		{
			IsGameOver = true;
			Players[CurrentPlayer]->OnWin();
			for (int32 i = 0; i < Players.Num(); i++)
			{
				if (i != CurrentPlayer)
				{
					Players[i]->OnLose();
				}
			}
		} 
		else
		{ */
			// reset attackable from fla

		// TODO => forse superfluo
		IsCheck();

		if (CheckFlag != EPawnColor::NONE)
			GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Yellow, FString::Printf(TEXT("King under check | %d"), CheckFlag));

		for (auto& InnerTArray : Players[FMath::Abs(CurrentPlayer - 1)]->AttackableTiles)
		{
			InnerTArray.Empty();
		} 
		Players[FMath::Abs(CurrentPlayer - 1)]->AttackableTiles.Empty();

		TArray<FTileSaving> BoardSaving;
		for (ATile* Tile : GField->GetTileArray())
		{
			// int8 PlayerColor = CurrentPlayer ? EPawnColor::BLACK : EPawnColor::WHITE;
			FTileStatus TileStatus = Tile->GetTileStatus();
			if (TileStatus.AttackableFrom[FMath::Abs(CurrentPlayer - 1)]) // reset attackable from of the opponent
			{
				TileStatus.AttackableFrom[FMath::Abs(CurrentPlayer - 1)] = false;
				Tile->SetTileStatus(TileStatus);
			}
		}

		for (auto& Piece : GField->PawnArray)
		{
			/* FTileSaving TileSaving = {
				Piece->GetPieceNum(),
				Piece->GetType(),
				Piece->GetColor(),
				Piece->GetStatus(),
				static_cast<int8>(Piece->GetGridPosition()[0]),
				static_cast<int8>(Piece->GetGridPosition()[1])
			}; */
			FTileSaving TileSaving = {
				static_cast<int8>(Piece->GetGridPosition()[0]),
				static_cast<int8>(Piece->GetGridPosition()[1]) 
			};
			BoardSaving.Add(TileSaving);
			
		}
		CurrentBoard = BoardSaving;
		GameSaving.Add(BoardSaving);



		TurnNextPlayer();
		// }



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


void AChess_GameMode::SetPawnPromotionChoice(EPawnType PawnType)
{
	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("AHIA %d %d"), LastGridPosition.X, LastGridPosition.Y));
	int8 X = LastGridPosition.X;
	int8 Y = LastGridPosition.Y;
	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("%d %d"), X, Y));
	PawnPromotionType = PawnType;




	// SPAWN & DISABLE PAWNS
	DespawnPawn(X, Y);
	ABasePawn* PawnTemp = SpawnPawn(PawnType, Players[CurrentPlayer]->Color, X, Y);
	ShowPossibleMoves(PawnTemp, true, true, false);
	if (PawnPromotionWidget != nullptr)
		PawnPromotionWidget->RemoveFromParent();

	// ComputeCheck();
	IsCheck();
	EndTurn(CurrentPlayer);
}

TArray<std::pair<int8, int8>> AChess_GameMode::ShowPossibleMoves(ABasePawn* Pawn, const bool CheckTest, const bool ShowAttackable, const bool CheckCheckFlag)
{
	TArray<std::pair<int8, int8>> PossibleMoves;
	if (Pawn != nullptr)
	{
		FVector2D CurrPawnGridPosition = Pawn->GetGridPosition();
		const int8 X = CurrPawnGridPosition[0];
		const int8 Y = CurrPawnGridPosition[1];

		TArray<ECardinalDirection> PawnDirections = Pawn->GetCardinalDirections();
		int8 MaxSteps = Pawn->GetMaxNumberSteps();
		int8 FlagDirection = 0;
		int8 XOffset = 0, YOffset = 0;
		for (const auto& PawnDirection : PawnDirections)
		{
			for (int8 i = 1; i <= MaxSteps; i++)
			{
				XOffset = 0, YOffset = 0;
				switch (PawnDirection)
				{
				case ECardinalDirection::NORTH:
					FlagDirection = 1;
				case ECardinalDirection::SOUTH:
					// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("FlagDirection %d"), FlagDirection));
					FlagDirection = FlagDirection ? FlagDirection : -1;
					XOffset = i * FlagDirection;
					YOffset = 0;
					break; 

				case ECardinalDirection::NORTHEAST:
					FlagDirection = 1;
				case ECardinalDirection::SOUTHWEST:
					FlagDirection = FlagDirection ? FlagDirection : -1;
					XOffset = i * FlagDirection;
					YOffset = i * FlagDirection;
					break;

				case ECardinalDirection::EAST:
					FlagDirection = 1;
				case ECardinalDirection::WEST:
					FlagDirection = FlagDirection ? FlagDirection : -1;
					XOffset = 0;
					YOffset = i * FlagDirection;
					break;

				case ECardinalDirection::NORTHWEST:
					FlagDirection = 1;
				case ECardinalDirection::SOUTHEAST: 
					FlagDirection = FlagDirection ? FlagDirection : -1;
					XOffset = i * FlagDirection;
					YOffset = i * (- FlagDirection);
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

				XOffset = XOffset * static_cast<int>(Pawn->GetColor());
				YOffset = YOffset * static_cast<int>(Pawn->GetColor());

			
				//ATile* Tile = GField->GetTileArray()[(NewX + XOffset) * GField->Size + NewY + YOffset];
				// bool EatFlag = Tile->GetTileStatus().PawnColor == EPawnColor::BLACK;
				if (IsValidMove(Pawn, X + XOffset, Y + YOffset, true, ShowAttackable, CheckCheckFlag))
				{
					PossibleMoves.Add(std::make_pair(X + XOffset, Y + YOffset));
				
					// avoid eating warning pawns on straight line
					if (ShowAttackable && !(Pawn->GetType() == EPawnType::PAWN && PawnDirection == ECardinalDirection::NORTH))
					{
						FTileStatus TileStatus = GField->GetTileArray()[(X + XOffset) * GField->Size + Y + YOffset]->GetTileStatus();
						TileStatus.AttackableFrom[(static_cast<int>(Pawn->GetColor()) == 1)? 0:1] = true; // idx 0 means attackable from whites, idx 1 means attackable from blacks 
						TileStatus.WhoCanGo.Add(Pawn); // TODO => NOT WORKING, used to compute correct move name
						GField->GetTileArray()[(X + XOffset) * GField->Size + Y + YOffset]->SetTileStatus(TileStatus); // TODO => player owner as ENUM
					}



					if (CurrentPlayer == 0 && !CheckTest)
					{
						UMaterialInterface* Material = ((X + XOffset + Y + YOffset) % 2) ? GField->MaterialLightRed : GField->MaterialDarkRed;
						GField->GetTileArray()[(X + XOffset) * GField->Size + Y + YOffset]->GetStaticMeshComponent()->SetMaterial(0, Material);

					}
				}
				FlagDirection = 0;
			
			}
		}
	}

	return PossibleMoves;
}

ABasePawn* AChess_GameMode::SpawnPawn(EPawnType PawnType, EPawnColor PawnColor, int8 X, int8 Y)
{
	TSubclassOf<ABasePawn> BasePawnClass;
	ABasePawn* BasePawnObj = nullptr;
	if (IsValidTile(X, Y))
	{
		ATile* TileObj = GField->GetTileArray()[X * GField->Size + Y];
		TileObj->SetPlayerOwner(CurrentPlayer);
		TArray<bool> TmpFalse; TmpFalse.Add(false); TmpFalse.Add(false);
		FTileStatus TileStatus = { 0, TmpFalse, TArray<ABasePawn*>(), EPawnColor::NONE, EPawnType::NONE };

		TSubclassOf<ABasePawn> W_PawnsClasses[] = { GField->W_RookClass, GField->W_KnightClass, GField->W_BishopClass, GField->W_QueenClass, GField->W_KingClass, GField->W_PawnClass };
		TSubclassOf<ABasePawn> B_PawnsClasses[] = { GField->B_RookClass, GField->B_KnightClass, GField->B_BishopClass, GField->B_QueenClass, GField->B_KingClass, GField->B_PawnClass };
		
		switch (PawnType)
		{
		case EPawnType::ROOK: 
			//BasePawnClass = W_RookClass; 
			TileStatus.PawnType = EPawnType::ROOK;
			break;
		case EPawnType::KNIGHT:
			//BasePawnClass = W_KnightClass;
			TileStatus.PawnType = EPawnType::KNIGHT;
			break;
		case EPawnType::BISHOP:
			//BasePawnClass = W_BishopClass; 
			TileStatus.PawnType = EPawnType::BISHOP;
			break;
		case EPawnType::QUEEN: 
			//BasePawnClass = W_QueenClass;
			TileStatus.PawnType = EPawnType::QUEEN;
			break;
		case EPawnType::KING: 
			//BasePawnClass = W_KingClass; 
			TileStatus.PawnType = EPawnType::KING;
			break;
		case EPawnType::PAWN: 
			//BasePawnClass = W_PawnClass;
			TileStatus.PawnType = EPawnType::PAWN;
			break;

		}

		// set material
		// UMaterialInterface* Material = nullptr;
		switch (PawnColor)
		{
		case EPawnColor::WHITE:
			TileStatus.PawnColor = EPawnColor::WHITE;
			BasePawnClass = W_PawnsClasses[static_cast<int>(PawnType) - 1];
			break;
		case EPawnColor::BLACK: 
			TileStatus.PawnColor = EPawnColor::BLACK;
			BasePawnClass = B_PawnsClasses[static_cast<int>(PawnType) - 1];
			break;
		}

		FVector Origin;
		FVector BoxExtent;
		TileObj->GetActorBounds(false, Origin, BoxExtent);
		FVector Location = GField->GetRelativeLocationByXYPosition(X, Y);
		FVector PawnLocation(Location.GetComponentForAxis(EAxis::X), Location.GetComponentForAxis(EAxis::Y), Location.GetComponentForAxis(EAxis::Z) + 2 * BoxExtent.GetComponentForAxis(EAxis::Z) + 0.1);

		BasePawnObj = GetWorld()->SpawnActor<ABasePawn>(BasePawnClass, PawnLocation, FRotator(0, 90, 0));
		// BasePawnObj->SetTileId(FString::Printf(TEXT("%c%d"), IdChar, IdNum));
		if (BasePawnObj != nullptr)
		{
			BasePawnObj->SetGridPosition(X, Y);
			// TODO: 0.8 da mettere come attributo
			const float TileScale = GField->TileSize / 100;
			BasePawnObj->SetActorScale3D(FVector(TileScale * 0.8, TileScale * 0.8, 0.03));

			BasePawnObj->SetType(TileStatus.PawnType);
			BasePawnObj->SetColor(TileStatus.PawnColor);
			BasePawnObj->SetStatus(EPawnStatus::ALIVE);

			GField->PawnArray.Add(BasePawnObj);
			GField->PawnMap.Add(FVector2D(X, Y), BasePawnObj);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ABasePawn Obj is null"));
		}

		TileObj->SetTileStatus(TileStatus);
		TileObj->SetPawn(BasePawnObj);
		

	}
	return BasePawnObj;

}

void AChess_GameMode::DespawnPawn(int8 X, int8 Y)
{
	if (IsValidTile(X, Y))
	{
		ATile* Tile = GField->GetTileArray()[X * GField->Size + Y];
		ABasePawn* Pawn = Tile->GetPawn();
		if (Tile != nullptr && Pawn != nullptr)
		{
			Tile->SetPawn(nullptr);
			TArray<bool> TmpFalse; TmpFalse.Add(false); TmpFalse.Add(false);
			Tile->SetTileStatus({ 1, TmpFalse, TArray<ABasePawn*>(), EPawnColor::NONE, EPawnType::NONE });
			Tile->SetPlayerOwner(-1);

			Pawn->SetStatus(EPawnStatus::DEAD);
			GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Blue, FString::Printf(TEXT("%f %f pawn has been eaten/despawned"), Pawn->GetGridPosition()[0], Pawn->GetGridPosition()[1]));
			// Hides visible components
			Pawn->SetActorHiddenInGame(true);

			// Disables collision components
			Pawn->SetActorEnableCollision(false);

			// Stops the Actor from ticking
			Pawn->SetActorTickEnabled(false);
		}
	}
}

EPawnColor AChess_GameMode::IsCheck(ABasePawn* Pawn, const int8 NewX, const int8 NewY)
{
	// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("IsCheck() - %d"), CurrentPlayer));
	UE_LOG(LogTemp, Warning, TEXT("Here i am in check"));


	// TArray<ATile*>
	
	EPawnColor ColorAttacker = CurrentPlayer ? EPawnColor::BLACK : EPawnColor::WHITE;	

	/* if (CheckFlag != EPawnColor::NONE && Pawn != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Yellow, FString::Printf(TEXT("BRO")));
	} */

	// check if king is under attack
	// if pawn != nullptr => it means that someone is under check
	if (Pawn == nullptr)
	{
		// TArray<ABasePawn*> MyPawns;
		bool BlackCanMove = false;
		bool WhiteCanMove = false;
		for (auto& CurrPawn : GField->GetPawnArray())
		{
			if (CurrPawn->GetStatus() == EPawnStatus::ALIVE) // TODO => solo del colore che difende (di pawn)
			{
				EPawnColor TmpCheckFlag = CheckFlag;

				// Obtain as TArray the possible tiles where CurrPawn can move itself
				TArray<std::pair<int8, int8>> Tmp = ShowPossibleMoves(CurrPawn, true, true, false);

				// Restore previous checkFlag
				CheckFlag = TmpCheckFlag;

				// If CurrPawn can make some eligible moves, it is pushed to MyPawns
				if (Tmp.Num() > 0)
				{
					// MyPawns.Add(CurrPawn);
					switch (CurrPawn->GetColor())
					{
					case EPawnColor::BLACK: BlackCanMove = true; break;
					case EPawnColor::WHITE: WhiteCanMove = true; break;
					}
				}
				
				// ShowPossibleMoves(CurrPawn, true, true, false);
			}
		}

		if (!BlackCanMove) CheckMateFlag = EPawnColor::BLACK;
		if (!WhiteCanMove) CheckMateFlag = EPawnColor::WHITE;
		
		if (CheckMateFlag == EPawnColor::NONE)
		{
			CheckFlag = EPawnColor::NONE;
			for (auto& CurrPawn : GField->GetPawnArray())
			{
				// TArray<FSteps> PossibleSteps;
				if (CurrPawn->GetType() == EPawnType::KING && CurrPawn->GetStatus() == EPawnStatus::ALIVE)
				{
					FVector2D PawnGrid = CurrPawn->GetGridPosition();
					int8 OpponentIdx = (static_cast<int>(CurrPawn->GetColor()) == 1) ? 1 : 0;
					if (GField->GetTileArray()[PawnGrid[0] * GField->Size + PawnGrid[1]]->GetTileStatus().AttackableFrom[OpponentIdx])
					{
						// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Yellow, FString::Printf(TEXT("King under check | %d"), CurrPawn->GetColor()));
						CheckFlag = CurrPawn->GetColor();
						break;
					}
					/* else
					{
						CheckFlag = EPawnColor::NONE;
					} */
				}
			}
		}
		return CheckFlag;
	}
	else
	{
		// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Yellow, FString::Printf(TEXT("Simulo mossa param")));
		
		if (IsValidMove(Pawn, NewX, NewY, true, false, false))
		{
			// update tile array


			FVector2D OldPosition = Pawn->GetGridPosition();
			ATile* OldTile = GField->GetTileArray()[OldPosition[0] * GField->Size + OldPosition[1]];
			ABasePawn* OldPawn = OldTile->GetPawn();
			int32 OldPlayerOwner = OldTile->GetPlayerOwner();
			FTileStatus OldTileStatus = OldTile->GetTileStatus();

			ATile* NewTile = GField->GetTileArray()[NewX * GField->Size + NewY];
			// ABasePawn* NewPawn = NewTile->GetPawn();
			int32 NewPlayerOwner = NewTile->GetPlayerOwner();
			FTileStatus NewTileStatus = NewTile->GetTileStatus();


			ABasePawn* PawnToEat = NewTile->GetPawn();
			EPawnStatus PawnToEatStatus;
			FVector2D PawnToEatGridPosition;
			if (PawnToEat != nullptr)
			{
				PawnToEatStatus = PawnToEat->GetStatus();
				PawnToEatGridPosition = PawnToEat->GetGridPosition();
				PawnToEat->SetStatus(EPawnStatus::DEAD);
				PawnToEat->SetGridPosition(-1, -1);
			}
			// TArray<std::pair<int8, int8>> PawnToEatAttackableTiles = ShowPossibleMoves(PawnToEat, true, false, false);
			TArray<std::pair<std::pair<int8, int8>, FTileStatus>> TileStatusBackup;
			for (ATile* Tile : GField->GetTileArray())
			{
				int8 X = Tile->GetGridPosition()[0];
				int8 Y = Tile->GetGridPosition()[1];
				FTileStatus TileStatus = Tile->GetTileStatus();
				TileStatusBackup.Add(std::make_pair(std::make_pair(X, Y), TileStatus));

				TArray<bool> TmpFalse;
				TmpFalse.Add(false); TmpFalse.Add(false);
				TileStatus.AttackableFrom = TmpFalse;
				TileStatus.WhoCanGo.Empty();
				Tile->SetTileStatus(TileStatus);
				/* if (PawnToEatAttackableTiles.Contains(std::pair<int8, int8>(Tile->GetGridPosition()[0], Tile->GetGridPosition()[1])))
				{
					int8 X = Tile->GetGridPosition()[0];
					int8 Y = Tile->GetGridPosition()[1];
					TileStatusBackup.Add(std::make_pair(std::make_pair(X, Y), Tile->GetTileStatus()));
				} */
			}

			OldTile->SetPawn(nullptr);
			OldTile->SetPlayerOwner(-1);
			TArray<bool> TmpFalse;
			TmpFalse.Add(false); TmpFalse.Add(false);
			OldTile->SetTileStatus({ 1, TmpFalse, TArray<ABasePawn*>(), EPawnColor::NONE, EPawnType::NONE });

			NewTile->SetPlayerOwner(CurrentPlayer);
			NewTile->SetTileStatus({ 0, TmpFalse, TArray<ABasePawn*>(), Pawn->GetColor(), Pawn->GetType() });
			NewTile->SetPawn(Pawn);
			Pawn->SetGridPosition(NewTile->GetGridPosition()[0], NewTile->GetGridPosition()[1]);


			// recalculate show possible moves
			// TODO => const auto& o solo auto&
			for (auto& PawnInArray : GField->GetPawnArray())
			{
				if (PawnInArray->GetStatus() == EPawnStatus::ALIVE) // TODO => solo del colore che difende (di pawn)
				{
					ShowPossibleMoves(PawnInArray, true, true, false);
				}
			}

			// ShowPossibleMoves(Pawn, true, false, false);


			EPawnColor OldCheckFlag = CheckFlag;
			EPawnColor NewCheckFlag = IsCheck();
			
			for (const auto& pair : TileStatusBackup)
			{
				GField->GetTileArray()[pair.first.first * GField->Size + pair.first.second]->SetTileStatus(pair.second);
			}

			// undo
			OldTile->SetPawn(OldPawn);
			OldTile->SetPlayerOwner(OldPlayerOwner);
			OldTile->SetTileStatus(OldTileStatus);

			NewTile->SetPawn(PawnToEat);
			NewTile->SetPlayerOwner(NewPlayerOwner);
			NewTile->SetTileStatus(NewTileStatus);
			Pawn->SetGridPosition(OldPosition[0], OldPosition[1]);

			if (PawnToEat != nullptr)
			{
				PawnToEat->SetStatus(PawnToEatStatus);
				PawnToEat->SetGridPosition(PawnToEatGridPosition[0], PawnToEatGridPosition[1]);
			}

			CheckFlag = OldCheckFlag;

			

			return NewCheckFlag;
		}
		return CheckFlag;

	}

	

	// return false;


	// <PAWN, <DELTAX, DELTAY>>
	// TArray<FPawnsPossibilites> PawnsAttackers;
	/* TArray<ABasePawn*> PawnsAttackers; // it contains pawn which can attack king
	for (auto& CurrPawn : GField->GetPawnArray())
	{
		// TArray<FSteps> PossibleSteps;
		if (CurrPawn->GetColor() == ColorAttacker && CurrPawn->GetStatus() == EPawnStatus::ALIVE)
		{
			TArray<std::pair<int8, int8>> PossibleMoves = ShowPossibleMoves(CurrPawn, true);
			for (auto& move : PossibleMoves)
			{
				ABasePawn* Pawn = GField->GetTileArray()[move.first * GField->Size + move.second]->GetPawn(); // prendo eventuale pawn nella cella attaccabile
				if (Pawn != nullptr && Pawn->GetColor() != ColorAttacker && Pawn->GetType() == EPawnType::KING)
				{
					PawnsAttackers.Add(CurrPawn);
					CheckFlag = Pawn->GetColor();
					GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("IsCheck() - %d"), CurrentPlayer)); GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("King under attack - %d"), CheckFlag));
					// break;
				}
			}
		}
	} */

	






	// for tutte le pedine del !current player
	//		check tutte le mosse possibili (ShowPossibleMoves(pawn, x, y))
	//			se una può mangiare re, SET EUNUM under_check { BLACK, WHITE, NONE }
	//			pedine_eating = {...} || se > 0 => esistono pedine che possono mangiare re
	//	show possible moves dipenderà da attributo under_check (se true, bisogna spostare il re o mangiare pedina che minaccia, ma dopo è necessario ricontrollare)

	// NON è possibile mangiare i re

	// return CheckFlag != EPawnColor::NONE;
}

// TODO make it const
bool AChess_GameMode::IsValidMove(ABasePawn* Pawn, const int8 NewX, const int8 NewY, /*const bool EatFlag,*/ const bool TestFlag, const bool ShowAttackable, const bool CheckCheckFlag)
{
	bool IsValid = false;

	if (Pawn != nullptr && IsValidTile(NewX, NewY)
		&& !(NewX == Pawn->GetGridPosition()[0] && NewY == Pawn->GetGridPosition()[1]))
	{
		TArray<ATile*> TileArray = GField->GetTileArray();
		ATile* NewTile = TileArray[NewX * GField->Size + NewY];
		// it means that colors are -1 and 1 or viceversa
		bool EatFlag = static_cast<int>(NewTile->GetTileStatus().PawnColor) == -static_cast<int>(Pawn->GetColor());
	

		// CurrTile => remove it

		FVector2D NewGridPosition = NewTile->GetGridPosition();
		FVector2D CurrGridPosition = Pawn->GetGridPosition();
	


		// NewGridPosition , CurrPawnPosition
		EPawnColor DirectionFlag = Pawn->GetColor();
		int8 DeltaX = (NewGridPosition[0] - CurrGridPosition[0]); //* static_cast<double>(DirectionFlag);
		int8 DeltaY = NewGridPosition[1] - CurrGridPosition[1];







		// TODO => se IsCheck e dopo la potenziale mossa è ancora IsCheck => mossa non valida (a meno di checkmate)



		// TODO => se il pawn è king e la cella è attackable from the opposite color, invalid move
		// EPawnColor CheckFlag; // which color is under check


		if ((NewTile->GetTileStatus().EmptyFlag && !EatFlag) || (EatFlag && !NewTile->GetTileStatus().EmptyFlag && (NewTile->GetTileStatus().PawnColor != Pawn->GetColor())))
		{
			
			switch (Pawn->GetType())
			{
			case EPawnType::PAWN:
				if (EatFlag || ShowAttackable)
					IsValid = this->CheckDirection(EDirection::DIAGONAL, Pawn, NewGridPosition, CurrGridPosition, TestFlag);
				else
					IsValid = this->CheckDirection(EDirection::FORWARD, Pawn, NewGridPosition, CurrGridPosition, TestFlag);
				break;

			case EPawnType::ROOK:
				IsValid = this->CheckDirection(EDirection::FORWARD, Pawn, NewGridPosition, CurrGridPosition);
				IsValid = IsValid || this->CheckDirection(EDirection::BACKWARD, Pawn, NewGridPosition, CurrGridPosition);
				IsValid = IsValid || this->CheckDirection(EDirection::HORIZONTAL, Pawn, NewGridPosition, CurrGridPosition);
				break;

			case EPawnType::KNIGHT:
				IsValid = this->CheckDirection(EDirection::KNIGHT, Pawn, NewGridPosition, CurrGridPosition);
				break;

			case EPawnType::BISHOP:
				IsValid = this->CheckDirection(EDirection::DIAGONAL, Pawn, NewGridPosition, CurrGridPosition);
				break;

			case EPawnType::QUEEN: // so it works like an OR for Queen and King
			case EPawnType::KING:
				// se king si vuole muovere in una pedina attaccabile da avversario, mossa non lecita
				// EPawnColor AttackableFrom = GField->GetTileArray()[NewGridPosition[0] * GField->Size + NewGridPosition[1]]->GetTileStatus().AttackableFrom;
				TArray<bool> AttackableFrom = GField->GetTileArray()[NewGridPosition[0] * GField->Size + NewGridPosition[1]]->GetTileStatus().AttackableFrom;
				// if (!(Pawn->GetType() == EPawnType::KING && AttackableFrom != EPawnColor::NONE && AttackableFrom != Pawn->GetColor()))
				
				if (!(Pawn->GetType() == EPawnType::KING && AttackableFrom[(static_cast<int>(Pawn->GetColor()) == 1) ? 1 : 0]))
				{
					IsValid = this->CheckDirection(EDirection::FORWARD, Pawn, NewGridPosition, CurrGridPosition);
					IsValid = IsValid || this->CheckDirection(EDirection::BACKWARD, Pawn, NewGridPosition, CurrGridPosition);
					IsValid = IsValid || this->CheckDirection(EDirection::HORIZONTAL, Pawn, NewGridPosition, CurrGridPosition);
					IsValid = IsValid || this->CheckDirection(EDirection::DIAGONAL, Pawn, NewGridPosition, CurrGridPosition);
				}
				break;
			}
		
		}

		if (CheckCheckFlag && IsValid /* && Pawn->GetColor() == CheckFlag && CheckFlag == ((CurrentPlayer) ? EPawnColor::BLACK : EPawnColor::WHITE */)
		{
			EPawnColor PreviousCheckFlag = CheckFlag;
			EPawnColor NewCheckFlag = IsCheck(Pawn, NewGridPosition[0], NewGridPosition[1]);
			CheckFlag = PreviousCheckFlag;
			// it must return new checkflag calculated with the new move passed as param
			// body => new tilearray, all false, then apply show possible moves del pawn in newtile
			// verify if king is attackable


			// terza condizione necessaria per evitare che un pawn metta in scacco la proprio squadra
			IsValid = NewCheckFlag == EPawnColor::NONE
				|| (CheckFlag == EPawnColor::NONE && NewCheckFlag == EPawnColor::BLACK && Pawn->GetColor() != EPawnColor::BLACK)
				|| (CheckFlag == EPawnColor::NONE && NewCheckFlag == EPawnColor::WHITE && Pawn->GetColor() != EPawnColor::WHITE); 
			// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Green, TEXT("ricalcolo ischeck"));

		}




	}

	return IsValid;
}



bool AChess_GameMode::CheckDirection(const EDirection Direction, ABasePawn* Pawn, const FVector2D NewGridPosition, const FVector2D CurrGridPosition, const bool TestFlag) const
{
	EPawnColor DirectionFlag = Pawn->GetColor();
	int8 DeltaX = (NewGridPosition[0] - CurrGridPosition[0]); // * static_cast<double>(DirectionFlag);
	int8 DeltaY = NewGridPosition[1] - CurrGridPosition[1];
	int8 MaxSteps = (Pawn->GetType() == EPawnType::PAWN) ? 1 : Pawn->GetMaxNumberSteps();

	switch (Direction)
	{
	case EDirection::FORWARD:
		if (DeltaY == 0 && (DeltaX* static_cast<double>(DirectionFlag)) >= 0 && (DeltaX* static_cast<double>(DirectionFlag)) <= Pawn->GetMaxNumberSteps())
		{
			if (!this->IsLineClear(ELine::VERTICAL, CurrGridPosition, DeltaX, DeltaY))
				return false;

			if (Pawn->GetType() == EPawnType::PAWN && !TestFlag)
				Pawn->SetMaxNumberSteps(1);
			
			return true;
		}
		break;

	case EDirection::BACKWARD:
		return DeltaY == 0 && ((- DeltaX)* static_cast<double>(DirectionFlag)) >= 0 && ((- DeltaX)* static_cast<double>(DirectionFlag)) <= Pawn->GetMaxNumberSteps() && IsLineClear(ELine::VERTICAL, CurrGridPosition, DeltaX, DeltaY);
		
	case EDirection::HORIZONTAL:
		return DeltaX == 0 && FMath::Abs(DeltaY) >= 0 && FMath::Abs(DeltaY) <= Pawn->GetMaxNumberSteps() && IsLineClear(ELine::HORIZONTAL, CurrGridPosition, DeltaX, DeltaY);

	case EDirection::DIAGONAL:
		if (FMath::Abs(DeltaX) == FMath::Abs(DeltaY) && FMath::Abs(DeltaX) <= MaxSteps)
		{
			if (!this->IsLineClear(ELine::DIAGONAL, CurrGridPosition, DeltaX, DeltaY))
				return false;

			if (Pawn->GetType() == EPawnType::PAWN && !TestFlag)
				Pawn->SetMaxNumberSteps(1);
			
			return true;
		}
		break;

	case EDirection::KNIGHT:
		return (FMath::Abs(DeltaX) == 1 && FMath::Abs(DeltaY) == 2) || (FMath::Abs(DeltaX) == 2 && FMath::Abs(DeltaY) == 1);
	}
	
	return false;
}

bool AChess_GameMode::IsValidTile(const int8 X, const int8 Y) const
{
	return X >= 0 && X < GField->Size
		&& Y >= 0 && Y < GField->Size;
}


void AChess_GameMode::ReplayMove(UTextBlock* TxtBlock)
{
	FString BtnName = TxtBlock->GetText().ToString();
	FString NumMoveStr;
	BtnName.Split(TEXT("."), &NumMoveStr, NULL);
	int8 NumMove = FCString::Atoi(*NumMoveStr);
	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("Replay mossa %d"), NumMove));

	
	if ((NumMove + 1) != MoveCounter)
	{
		CanPlay = false;
		// TODO => disable game (Flag in GameMode => CanPlay)
		
		// show board to load


		// restore
		TArray<FTileSaving> BoardToLoad = GameSaving[NumMove - 1];
		GField->LoadBoard(BoardToLoad, false);
		

	}
	else
	{
		// TODO => usare delegati per riprendere il  game quando clicco su turno attuale
		GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("Bro cosa stai cercando")));
		GField->LoadBoard(CurrentBoard, true);
		CanPlay = true;
		Players[CurrentPlayer]->OnTurn();
	}





}

void AChess_GameMode::AddToReplay(const ABasePawn* Pawn, const bool EatFlag)
{
	FString MoveStr = ComputeMoveName(Pawn, EatFlag);

	// Update RecordMoves
	RecordMoves.Add(MoveStr);

	// Update Replay widget content
	UWorld* World = GetWorld();
	// UUniformGridPanel* UniformGridPanel = Cast<UUniformGridPanel>(ReplayWidget->GetWidgetFromName(TEXT("ufg_Replay")));
	UScrollBox* ScrollBox = Cast<UScrollBox>(ReplayWidget->GetWidgetFromName(TEXT("scr_Replay")));
	if (World && /* UniformGridPanel  && */ ScrollBox && ButtonWidgetRef)
	{
		UUserWidget* WidgetBtn = CreateWidget(World, ButtonWidgetRef);
		if (WidgetBtn)
		{
			UTextBlock* BtnText = Cast<UTextBlock>(WidgetBtn->GetWidgetFromName(TEXT("txtBlock")));
			if (BtnText)
			{
				// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("setto"));
				BtnText->SetText(FText::FromString(FString::Printf(TEXT("%d. %s"), MoveCounter, *MoveStr)));
			}

			FWidgetTransform Transform; 
			Transform.Angle = 180;
			WidgetBtn->SetRenderTransform(Transform);
			UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(ScrollBox->AddChild(WidgetBtn));
			ScrollSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
			ScrollSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Top);
			ScrollBox->ScrollToEnd();
		}
	}
}

FString AChess_GameMode::ComputeMoveName(const ABasePawn* Pawn, const bool EatFlag) const
{
	FVector2D GridPosition = Pawn->GetGridPosition();
	ATile* Tile = GField->GetTileArray()[GridPosition[0] * GField->Size + GridPosition[1]];
	ATile* PreviousTile = GField->GetTileArray()[PreviousGridPosition[0] * GField->Size + PreviousGridPosition[1]];
	bool IsPawn = Pawn->GetType() == EPawnType::PAWN;
	
	FString MoveStr = TEXT("");

	if (Pawn && !Tile->GetId().IsEmpty())
	{
		MoveStr = (IsPawn? TEXT("") : Pawn->GetId()) +
			((IsPawn && EatFlag)? PreviousTile->GetLetterId().ToLower() : TEXT("")) +
			(EatFlag ? TEXT("x") : TEXT("")) + 
			Tile->GetId().ToLower() +
			((CheckFlag != EPawnColor::NONE &&  CheckMateFlag == EPawnColor::NONE)? TEXT("+") : TEXT("")) +
			(CheckMateFlag != EPawnColor::NONE ? TEXT("#") : TEXT(""));

		GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, MoveStr);
	}

	return MoveStr;
}


/* void AChess_GameMode::ComputeCheck()
{
	for (const auto& Pawn : GField->PawnArray)
	{
		if (Pawn->GetStatus() == EPawnStatus::ALIVE)
			ShowPossibleMoves(Pawn, true, false, false);
	}
	IsCheck();
} */

/*
* return false if there is a pawn along the movement
*/
bool AChess_GameMode::IsLineClear(ELine Line, const FVector2D CurrGridPosition, const int8 DeltaX, const int8 DeltaY) const
{
	switch (Line)
	{
	case ELine::HORIZONTAL:	
		for (int8 YOffset = 1; YOffset < FMath::Abs(DeltaY); YOffset++)
		{
			if (IsValidTile(CurrGridPosition[0], CurrGridPosition[1] + YOffset * FMath::Sign(DeltaY))
				&& !GField->TileArray[CurrGridPosition[0] * GField->Size + CurrGridPosition[1] + YOffset * FMath::Sign(DeltaY)]->GetTileStatus().EmptyFlag)
				return false;
		}
		break;

	case ELine::VERTICAL:
		for (int8 XOffset = 1; XOffset < FMath::Abs(DeltaX); XOffset++)
		{
			if (IsValidTile(CurrGridPosition[0] + XOffset * FMath::Sign(DeltaX), CurrGridPosition[1])
				&& !GField->TileArray[(CurrGridPosition[0] + XOffset * FMath::Sign(DeltaX)) * GField->Size + CurrGridPosition[1]]->GetTileStatus().EmptyFlag)
				return false;
		}
		break;

	case ELine::DIAGONAL:
		for (int8 Offset = 1; Offset < FMath::Abs(DeltaX); Offset++)
		{
			if (IsValidTile(CurrGridPosition[0] + Offset * FMath::Sign(DeltaX), CurrGridPosition[1] + Offset * FMath::Sign(DeltaY))
				&& !GField->TileArray[(CurrGridPosition[0] + Offset * FMath::Sign(DeltaX)) * GField->Size + CurrGridPosition[1] + Offset * FMath::Sign(DeltaY)]->GetTileStatus().EmptyFlag)
				return false;
		}
		break; 
	}

	return true;
}