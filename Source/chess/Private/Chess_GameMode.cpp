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

	// Init variables
	IsGameOver = false;
	CanPlay = true;
	MoveCounter = 0;
	CheckFlag = EPawnColor::NONE;
	CheckMateFlag = EPawnColor::NONE;

	// Spawn GameField
	if (GameFieldClass)
	{
		GField = GetWorld()->SpawnActor<AGameField>(GameFieldClass);
		GField->Size = FieldSize;

		// Get and Set Human Player (Camera) Location
		AChess_HumanPlayer* HumanPlayer = Cast<AChess_HumanPlayer>(*TActorIterator<AChess_HumanPlayer>(GetWorld()));
		float CameraPosX = ((GField->TileSize * (FieldSize + ((FieldSize - 1) * GField->NormalizedCellPadding) - (FieldSize - 1))) / 2) - (GField->TileSize / 2);
		FVector CameraPos(CameraPosX, CameraPosX, 1250.0f); // TODO: 1000 da mettere come attributo
		HumanPlayer->SetActorLocationAndRotation(CameraPos, FRotationMatrix::MakeFromX(FVector(0, 0, -1)).Rotator());
	
		// Human player at INDEX 0
		Players.Add(HumanPlayer);

		// Random player
		auto* AI = GetWorld()->SpawnActor<AChess_RandomPlayer>(FVector(), FRotator());
		// Minimax player
		// auto* AI = GetWorld()->SpawnActor<AChess_MiniMaxPlayer>(FVector(), FRotator());

		Players.Add(AI);

		// Create replay widget
		UWorld* World = GetWorld();
		if (World && ReplayWidgetRef)
		{
			ReplayWidget = CreateWidget<UUserWidget>(World, ReplayWidgetRef, FName("Replay"));
			ReplayWidget->AddToViewport(0);
		}

		// Ready to start the game
		this->ChoosePlayerAndStartGame();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameFieldClass is null"));
	}
}

/*
 * Function: ChoosePlayerAndStartGame
 * ----------------------------
 *   Assigning player numbers and colors and starts the game
 */
void AChess_GameMode::ChoosePlayerAndStartGame()
{
	CurrentPlayer = 0;
	for (int32 i = 0; i < Players.Num(); i++)
	{
		Players[i]->PlayerNumber = i;
		Players[i]->Color = i == CurrentPlayer ? EPawnColor::WHITE : EPawnColor::BLACK;
	}

	CheckFlag = EPawnColor::NONE;
	CheckMateFlag = EPawnColor::NONE;
	MoveCounter += 1;
	CanPlay = true;

	InitTurn();

	Players[FMath::Abs(CurrentPlayer - 1)]->IsMyTurn = false;
	Players[CurrentPlayer]->IsMyTurn = true;
	Players[CurrentPlayer]->OnTurn();
}

/*
 * Function: EndTurn
 * ----------------------------
 *   Handles the end of turn of the player passed as parameter
 *
 *   PlayerNumber	int32	The number of the player whose turn ends
 */
void AChess_GameMode::EndTurn(const int32 PlayerNumber)
{

	// -1 to notify checkmate
	if (PlayerNumber == -1)
	{
		IsGameOver = true;

		GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("%d under CHECK MATE"), CheckMateFlag));

		// On Win and On Lose events
		Players[CurrentPlayer]->OnWin();
		for (int32 i = 0; i < Players.Num(); i++)
			if (i != CurrentPlayer)
				Players[i]->OnLose();
	
		// Timer to reset the field
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


		Players[PlayerNumber]->IsMyTurn = false;

		// TODO => forse superfluo
		IsCheck();


		if (CheckFlag != EPawnColor::NONE)
			GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Yellow, FString::Printf(TEXT("King under check | %d"), CheckFlag));

		// Clean opponent's attackable tiles, they will be overwritten the next turn,
		// so the previous state is useless
		for (auto& InnerTArray : Players[FMath::Abs(CurrentPlayer - 1)]->AttackableTiles)
		{
			InnerTArray.Empty();
		} 
		Players[FMath::Abs(CurrentPlayer - 1)]->AttackableTiles.Empty();

		
		
		for (ATile* Tile : GField->GetTileArray())
		{
			FTileStatus TileStatus = Tile->GetTileStatus();
			TileStatus.AttackableFrom[FMath::Abs(CurrentPlayer - 1)] = false;
			TileStatus.WhoCanGo.Empty();
			Tile->SetTileStatus(TileStatus);
			/* if (TileStatus.AttackableFrom[FMath::Abs(CurrentPlayer - 1)]) // reset attackable from of the opponent
			{
				TileStatus.AttackableFrom[FMath::Abs(CurrentPlayer - 1)] = false;
				Tile->SetTileStatus(TileStatus);
			} */
		}


		// Save position (X,Y) of each piece in order to store current board situation
		//	the index in the BoardSaving TArray identifies each piece
		//	e.g. White rook at bottom-left is at index 0,
		//	     White knight next to it is at 1,
		//		 ...
		// e.g. [{0,0}, {2,0}, {1,3}, ...] means the Bottom-left white rook is at (0,0), the knight next to it is at (2,0), ...
		TArray<FTileSaving> BoardSaving;
		for (auto& Piece : GField->PawnArray)
		{
			BoardSaving.Add({
				static_cast<int8>(Piece->GetGridPosition()[0]),
				static_cast<int8>(Piece->GetGridPosition()[1]),
				Piece->GetStatus()
			});
			
		}

		// Save current board and add it to the game history
		CurrentBoard = BoardSaving;
		GameSaving.Add(BoardSaving);

		TurnNextPlayer();
	}
}

/*
 * Function: GetNextPlayer
 * ----------------------------
 *   Returns next player index
 *
 *   Player		int32	Number of the current player
 * 
 *   return: next player index
 */
int32 AChess_GameMode::GetNextPlayer(int32 Player)
{
	// TODO => sostituire con FMath::Abs(Player - 1)
	Player++;
	if (!Players.IsValidIndex(Player))
	{
		Player = 0;
	}
	return Player;
}

/*
 * Function: TurnNextPlayer
 * ----------------------------
 *   Starts the next player turn
 */
void AChess_GameMode::TurnNextPlayer()
{
	MoveCounter += 1;
	CurrentPlayer = GetNextPlayer(CurrentPlayer);

	InitTurn();

	Players[CurrentPlayer]->IsMyTurn = true;
	Players[CurrentPlayer]->OnTurn();
}

void AChess_GameMode::InitTurn()
{
	// Clear possible moves
	for (auto& InnerArray : TurnPossibleMoves)
		InnerArray.Empty();
	TurnPossibleMoves.Empty();

	// Clear tile status (attackable from and who can go variables)
	for (auto& Tile : GField->GetTileArray())
	{
		FTileStatus TileStatus = Tile->GetTileStatus();
		TileStatus.AttackableFrom.SetNum(2, false);
		TileStatus.WhoCanGo.Empty();
		Tile->SetTileStatus(TileStatus);
	}

	WhitePiecesCanMove.Empty();
	BlackPiecesCanMove.Empty();


	// TODO => compute possible moves of current player 
	// (tarray di tarray per mosse possibli, accedo con pieceNum
	bool BlackCanMove = false;
	bool WhiteCanMove = false;
	for (auto& Piece : GField->PawnArray)
	{
		EPawnColor PreviousCheckFlag = CheckFlag;
		TArray<std::pair<int8, int8>> Tmp = ShowPossibleMoves(Piece, false, true, true);
		TurnPossibleMoves.Add(Tmp);
		CheckFlag = PreviousCheckFlag;
		if (Tmp.Num() > 0)
		{
			switch (Piece->GetColor())
			{
			case EPawnColor::WHITE: WhitePiecesCanMove.Add(Piece->GetPieceNum()); WhiteCanMove = true; break;
			case EPawnColor::BLACK: BlackPiecesCanMove.Add(Piece->GetPieceNum()); BlackCanMove = true; break;
			}
		}
		
	}

	/* if (!WhiteCanMove || !BlackCanMove)
	{
		GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("Someone cannot move anything"));
		EndTurn(-1);
	} */
}

/*
 * Function: SetPawnPromotionChoice
 * ----------------------------
 *   Promote the pawn with a new chess piece (type passed as parameter)
 *
 *   PawnType	EPawnType	: type of the new chess piece to spawn
 */
void AChess_GameMode::SetPawnPromotionChoice(EPawnType PawnType)
{	
	int8 X = LastGridPosition.X;
	int8 Y = LastGridPosition.Y;
	bool EatFlag = GField->TileArray[X * GField->Size + Y]->GetPawn() ? true : false;
	PawnPromotionType = PawnType;

	// Despawn & Spawn pawn
	GField->DespawnPawn(X, Y);
	ABasePawn* PawnTemp = GField->SpawnPawn(PawnType, Players[CurrentPlayer]->Color, X, Y);
	// Calculate new chess piece eligible moves
	// TODO => superfluo, già fatto in ischeck
	//ShowPossibleMoves(PawnTemp, true, false);

	if (PawnPromotionWidget != nullptr)
		PawnPromotionWidget->RemoveFromParent();

	// TODO => vedere se si può alleggerire (posso rimuovere, viene fatto quando l'ai inizia il turno)
	IsCheck();
	AddToReplay(PawnTemp, EatFlag);
	EndTurn(CurrentPlayer);
}

/*
 * Function: ShowPossibleMoves
 * ----------------------------
 *   Calculates the eligible moves of the chess piece passed as parameter
 *
 *   Pawn			ABasePawn*	: pawn on which to calculate the eligible moves
 *	 ShowAttackable	bool = false: flag to determine if only the attackable tiles should be taken into account 
									(possible tiles to move on and attackable tiles are different for pawns)
 *	 CheckCheckFlag	bool = false: flag to determine if checking the new check situation should be evaluated
 *
 *   return: TArray made of new possible X,Y of the chess piece
 */
TArray<std::pair<int8, int8>> AChess_GameMode::ShowPossibleMoves(ABasePawn* Pawn, const bool ShowAttackable, const bool CheckCheckFlag, const bool UpdateWhoCanGoFlag)
{
	TArray<std::pair<int8, int8>> PossibleMoves;
	if (Pawn && Pawn->GetStatus() == EPawnStatus::ALIVE)
	{
		FVector2D CurrPawnGridPosition = Pawn->GetGridPosition();
		const int8 X = CurrPawnGridPosition[0];
		const int8 Y = CurrPawnGridPosition[1];

		TArray<ECardinalDirection> PawnDirections = Pawn->GetCardinalDirections();
		int8 MaxSteps = Pawn->GetMaxNumberSteps();
		int8 XOffset = 0, YOffset = 0;
		for (const auto& PawnDirection : PawnDirections)
		{
			for (int8 i = 1; i <= MaxSteps; i++)
			{
				std::pair<int8, int8> Offsets = GetXYOffset(i, PawnDirection, Pawn->GetColor());
				XOffset = Offsets.first;
				YOffset = Offsets.second;

				// Evaluate if this move is valid or not
				if (IsValidMove(Pawn, X + XOffset, Y + YOffset, true, ShowAttackable, CheckCheckFlag))
				{
					// Add the VALID move to result TArray
					PossibleMoves.Add(std::make_pair(X + XOffset, Y + YOffset));
					FTileStatus TileStatus = GField->GetTileArray()[(X + XOffset) * GField->Size + Y + YOffset]->GetTileStatus();
				
					// Check needed to avoid pawns eat on straight line
					if (ShowAttackable && !(Pawn->GetType() == EPawnType::PAWN && PawnDirection == ECardinalDirection::NORTH))
					{
						// Index 0 means attackable from whites
						// Index 1 means attackable from blacks 
						TileStatus.AttackableFrom[(static_cast<int>(Pawn->GetColor()) == 1)? 0:1] = true; 
					}
					if (UpdateWhoCanGoFlag)
						TileStatus.WhoCanGo.Add(Pawn); // TODO => NOT WORKING, used to compute correct move name

					GField->GetTileArray()[(X + XOffset) * GField->Size + Y + YOffset]->SetTileStatus(TileStatus); // TODO => player owner as ENUM
				}
			}
		}
		
		// Castling handling
		/* if (Pawn->GetType() == EPawnType::KING)
		{
			// Short castling
			std::pair<int8, int8> Offsets = GetXYOffset(2, ECardinalDirection::EAST, Pawn->GetColor());
			XOffset = Offsets.first;
			YOffset = Offsets.second;
			// Evaluate if this move is valid or not
			if (IsValidMove(Pawn, X + XOffset, Y + YOffset, true, ShowAttackable, CheckCheckFlag))
			{
				// Add the VALID move to result TArray
				PossibleMoves.Add(std::make_pair(X + XOffset, Y + YOffset));
			}


			// Long castling
			// std::pair<int8, int8> Offsets = GetXYOffset(2, ECardinalDirection::WEST, Pawn->GetColor());
		} */
	}

	return PossibleMoves;
}


/*
 * Function: IsCheck
 * ----------------------------
 *   Computes (if no parameters are passed) and determines if a king is under king
 * 
 *	TODO => descrivere algoritmo
 *
 *	 Pawn	ABasePawn* = nullptr	: pawn to move on new x and new y and compute check situation
 *   NewX	const int8 = -1			: eventually new x position of the pawn
 *	 NewY	const int8 = -1			: eventually new y position of the pawn
 *
 *   return: Pointer to the recently spawned pawn
 */
EPawnColor AChess_GameMode::IsCheck(ABasePawn* Pawn, const int8 NewX, const int8 NewY)
{
	EPawnColor ColorAttacker = CurrentPlayer ? EPawnColor::BLACK : EPawnColor::WHITE;	

	// check if king is under attack
	// if pawn != nullptr => it means that someone is under check
	if (Pawn == nullptr)
	{
		ComputeAttackableTiles();
		
		if (CheckMateFlag == EPawnColor::NONE)
		{
			CheckFlag = CheckKingUnderAttack();
		}
		return CheckFlag;
	}
	else
	{
		// try to do move passed as parameter
		if (GField->IsValidTile(NewX, NewY)
			&& IsValidMove(Pawn, NewX, NewY, true, false, false))
		{

			// Backup old tile and new tile
			FVector2D Backup_OldPosition = Pawn->GetGridPosition();
			ATile* OldTile = GField->GetTileArray()[Backup_OldPosition[0] * GField->Size + Backup_OldPosition[1]];
			ATile* NewTile = GField->GetTileArray()[NewX * GField->Size + NewY];

			ABasePawn* Backup_OldPawn = OldTile->GetPawn();
			int32 Backup_OldPlayerOwner = OldTile->GetPlayerOwner();
			FTileStatus Backup_OldTileStatus = OldTile->GetTileStatus();

			int32 Backup_NewPlayerOwner = NewTile->GetPlayerOwner();
			FTileStatus Backup_NewTileStatus = NewTile->GetTileStatus();

			ABasePawn* PawnToEat = NewTile->GetPawn();
			EPawnStatus Backup_PawnToEatStatus;
			FVector2D Backup_PawnToEatGridPosition;
			if (PawnToEat)
			{
				Backup_PawnToEatStatus = PawnToEat->GetStatus();
				Backup_PawnToEatGridPosition = PawnToEat->GetGridPosition();
				PawnToEat->SetStatus(EPawnStatus::DEAD);
				PawnToEat->SetGridPosition(-1, -1);
			}

			// Backup tile status
			TArray<std::pair<std::pair<int8, int8>, FTileStatus>> TileStatusBackup;
			for (ATile* Tile : GField->GetTileArray())
			{
				int8 X = Tile->GetGridPosition()[0];
				int8 Y = Tile->GetGridPosition()[1];
				FTileStatus TileStatus = Tile->GetTileStatus();
				TileStatusBackup.Add(std::make_pair(std::make_pair(X, Y), TileStatus));

				TileStatus.AttackableFrom.SetNum(2, false);
				// TileStatus.WhoCanGo.Empty();
				Tile->SetTileStatus(TileStatus);
			}

			// Simulate move
			Pawn->Move(OldTile, NewTile, true);

			// Compute attackable tile with the new move
			ComputeAttackableTiles();

			EPawnColor OldCheckFlag = CheckFlag;
			EPawnColor NewCheckFlag = IsCheck();

			// undo | restore backedup info
			for (const auto& pair : TileStatusBackup)
				GField->GetTileArray()[pair.first.first * GField->Size + pair.first.second]->SetTileStatus(pair.second);
			
			OldTile->SetPawn(Backup_OldPawn);
			OldTile->SetPlayerOwner(Backup_OldPlayerOwner);
			OldTile->SetTileStatus(Backup_OldTileStatus);

			NewTile->SetPawn(PawnToEat);
			NewTile->SetPlayerOwner(Backup_NewPlayerOwner);
			NewTile->SetTileStatus(Backup_NewTileStatus);
			Pawn->SetGridPosition(Backup_OldPosition[0], Backup_OldPosition[1]);

			if (PawnToEat)
			{
				PawnToEat->SetStatus(Backup_PawnToEatStatus);
				PawnToEat->SetGridPosition(Backup_PawnToEatGridPosition[0], Backup_PawnToEatGridPosition[1]);
			}

			CheckFlag = OldCheckFlag;

			return NewCheckFlag;
		}
		return CheckFlag;
	}
}


void AChess_GameMode::ComputeAttackableTiles()
{
	bool BlackCanMove = false;
	bool WhiteCanMove = false;


	for (auto& Tile : GField->TileArray)
	{
		FTileStatus TileStatus = Tile->GetTileStatus();
		TArray<bool> TmpFalse; TmpFalse.SetNum(2, false);
		TileStatus.AttackableFrom = TmpFalse;
		Tile->SetTileStatus(TileStatus);
	}

	for (auto& CurrPawn : GField->GetPawnArray())
	{
		if (CurrPawn->GetStatus() == EPawnStatus::ALIVE) 
		{
			// Obtain as TArray the possible tiles where CurrPawn can move itself
			TArray<std::pair<int8, int8>> Tmp = ShowPossibleMoves(CurrPawn, true, false);

			if (Tmp.Num() > 0)
			{
				switch (CurrPawn->GetColor())
				{
				case EPawnColor::BLACK: BlackCanMove = true; break;
				case EPawnColor::WHITE: WhiteCanMove = true; break;
				}
			}
		}
	}

	if (!BlackCanMove) CheckMateFlag = EPawnColor::BLACK;
	if (!WhiteCanMove) CheckMateFlag = EPawnColor::WHITE;
}

EPawnColor AChess_GameMode::CheckKingUnderAttack() const
{
	EPawnColor TmpCheckFlag = EPawnColor::NONE;
	for (auto& Piece : GField->GetPawnArray())
	{
		if (Piece->GetType() == EPawnType::KING && Piece->GetStatus() == EPawnStatus::ALIVE)
		{
			FVector2D PawnGrid = Piece->GetGridPosition();
			int8 OpponentIdx = (static_cast<int>(Piece->GetColor()) == 1) ? 1 : 0;
			if (GField->GetTileArray()[PawnGrid[0] * GField->Size + PawnGrid[1]]->GetTileStatus().AttackableFrom[OpponentIdx])
			{
				TmpCheckFlag = Piece->GetColor();
				break;
			}
		}
	}
	return TmpCheckFlag;
}


// TODO make it const
/*
 * Function: IsValidMove
 * ----------------------------
 *   Computes if a move (specified through parameters) is valid or not
 * 
 *	TODO => descrivere algoritmo
 *
 *	 Pawn				ABasePawn*			: pawn to move on new x and new y and compute check situation
 *   NewX				const int8			: new x position of the piece
 *	 NewY				const int8			: new y position of the piece
 *	 TestFlag	TODO => forse superfluo		const bool = false	: flag which identifies if this function is called as test or effective move
 *	 ShowAttackable	bool = false: flag to determine if only the attackable tiles should be taken into account 
									(possible tiles to move on and attackable tiles are different for pawns)
 *	 CheckCheckFlag		const bool = true	: flag to determine if checking check state after this move is required
 * 
 *   return 	bool	determines if a move is valid or not
 */
bool AChess_GameMode::IsValidMove(ABasePawn* Pawn, const int8 NewX, const int8 NewY, const bool TestFlag, const bool ShowAttackable, const bool CheckCheckFlag)
{
	bool IsValid = false;

	if (Pawn 
		&& GField->IsValidTile(NewX, NewY)
		&& !(NewX == Pawn->GetGridPosition()[0] && NewY == Pawn->GetGridPosition()[1]))
	{
		ATile* NewTile = GField->TileArray[NewX * GField->Size + NewY];

		// it means that colors of the pieces on the tile are -1 and 1 or viceversa, 
		// if so, a piece is eating another one
		bool EatFlag = static_cast<int>(NewTile->GetTileStatus().PawnColor) == -static_cast<int>(Pawn->GetColor());
	
		FVector2D NewGridPosition = NewTile->GetGridPosition();
		FVector2D CurrGridPosition = Pawn->GetGridPosition();
	
		EPawnColor DirectionFlag = Pawn->GetColor();
		int8 DeltaX = (NewGridPosition[0] - CurrGridPosition[0]); 
		int8 DeltaY = NewGridPosition[1] - CurrGridPosition[1];



		// TODO => se IsCheck e dopo la potenziale mossa è ancora IsCheck => mossa non valida (a meno di checkmate)

		// TODO => se il pawn è king e la cella è attackable from the opposite color, invalid move
		// EPawnColor CheckFlag; // which color is under check

		// TODO => forse condizione già gestite in validMove()
		if ((NewTile->GetTileStatus().EmptyFlag && !EatFlag) 
			|| (EatFlag && !NewTile->GetTileStatus().EmptyFlag && (NewTile->GetTileStatus().PawnColor != Pawn->GetColor())))
		{
			// Perform direction check based on directions which the piece can perform
			switch (Pawn->GetType())
			{
			case EPawnType::PAWN:
				if (EatFlag || ShowAttackable)
					IsValid = GField->CheckDirection(EDirection::DIAGONAL, Pawn, NewGridPosition, CurrGridPosition, TestFlag);
				else
					IsValid = GField->CheckDirection(EDirection::FORWARD, Pawn, NewGridPosition, CurrGridPosition, TestFlag);
				break;

			case EPawnType::ROOK:
				IsValid = GField->CheckDirection(EDirection::FORWARD, Pawn, NewGridPosition, CurrGridPosition);
				IsValid = IsValid || GField->CheckDirection(EDirection::BACKWARD, Pawn, NewGridPosition, CurrGridPosition);
				IsValid = IsValid || GField->CheckDirection(EDirection::HORIZONTAL, Pawn, NewGridPosition, CurrGridPosition);
				break;

			case EPawnType::KNIGHT:
				IsValid = GField->CheckDirection(EDirection::KNIGHT, Pawn, NewGridPosition, CurrGridPosition);
				break;

			case EPawnType::BISHOP:
				IsValid = GField->CheckDirection(EDirection::DIAGONAL, Pawn, NewGridPosition, CurrGridPosition);
				break;

			case EPawnType::QUEEN: // so it works like an OR for Queen and King
			case EPawnType::KING:
				// se king si vuole muovere in una pedina attaccabile da avversario, mossa non lecita
				TArray<bool> AttackableFrom = GField->GetTileArray()[NewGridPosition[0] * GField->Size + NewGridPosition[1]]->GetTileStatus().AttackableFrom;
				
				// If the piece is the king and it is attackable from the opponent player,
				//	IsValid flag is determined SOMEWHERE ELSE (wHERE ?? TODO )
				if (!(Pawn->GetType() == EPawnType::KING && AttackableFrom[(static_cast<int>(Pawn->GetColor()) == 1) ? 1 : 0]))
				{
					IsValid = GField->CheckDirection(EDirection::FORWARD, Pawn, NewGridPosition, CurrGridPosition);
					IsValid = IsValid || GField->CheckDirection(EDirection::BACKWARD, Pawn, NewGridPosition, CurrGridPosition);
					IsValid = IsValid || GField->CheckDirection(EDirection::HORIZONTAL, Pawn, NewGridPosition, CurrGridPosition);
					IsValid = IsValid || GField->CheckDirection(EDirection::DIAGONAL, Pawn, NewGridPosition, CurrGridPosition);
				}
				break;
			}
		}

		// If the move is valid and checking the new check state is required,
		//	computing the new check situation
		if (IsValid && CheckCheckFlag)
		{
			EPawnColor PreviousCheckFlag = CheckFlag;
			// Check state after having moved the Piece in the new grid position
			if (GField->IsValidTile(NewGridPosition[0], NewGridPosition[1]) && Pawn->GetStatus() == EPawnStatus::ALIVE)
			{
				EPawnColor NewCheckFlag = IsCheck(Pawn, NewGridPosition[0], NewGridPosition[1]);
				CheckFlag = PreviousCheckFlag;

				// Possible valid situations after having checked the new check situation
				IsValid = NewCheckFlag == EPawnColor::NONE
					|| (CheckFlag == EPawnColor::NONE && NewCheckFlag == EPawnColor::BLACK && Pawn->GetColor() != EPawnColor::BLACK)
					|| (CheckFlag == EPawnColor::NONE && NewCheckFlag == EPawnColor::WHITE && Pawn->GetColor() != EPawnColor::WHITE); 
			}
		}
	}

	return IsValid;
}



/*
 * Function: GetXYOffset
 * ----------------------------
 *   Computes the offsets (X,Y) based on the paramaters (steps, direction and color)
 *
 *	 Steps		const int8				number of steps to perform
 *	 Direction	ECardinalDirection		direction to follow during the move
 *	 PieceColor	EPawnColor				color of the piece
 *	 
 *   return		std::pair<int8, int8>	pair containing XOffset as first and YOffset as second
 */
std::pair<int8, int8> AChess_GameMode::GetXYOffset(const int8 Steps, const ECardinalDirection Direction, const EPawnColor PieceColor) const
{
	// Flag to determine if going to north or south / east or west / ... / and exploit symmetry
	int8 FlagDirection = 0; 
	int8 XOffset = 0, YOffset = 0;
	
	switch (Direction)
	{
	case ECardinalDirection::NORTH:
		FlagDirection = 1;
	case ECardinalDirection::SOUTH:
		FlagDirection = FlagDirection ? FlagDirection : -1;
		XOffset = Steps * FlagDirection;
		YOffset = 0;
		break;

	case ECardinalDirection::NORTHEAST:
		FlagDirection = 1;
	case ECardinalDirection::SOUTHWEST:
		FlagDirection = FlagDirection ? FlagDirection : -1;
		XOffset = Steps * FlagDirection;
		YOffset = Steps * FlagDirection;
		break;

	case ECardinalDirection::EAST:
		FlagDirection = 1;
	case ECardinalDirection::WEST:
		FlagDirection = FlagDirection ? FlagDirection : -1;
		XOffset = 0;
		YOffset = Steps * FlagDirection;
		break;

	case ECardinalDirection::NORTHWEST:
		FlagDirection = 1;
	case ECardinalDirection::SOUTHEAST:
		FlagDirection = FlagDirection ? FlagDirection : -1;
		XOffset = Steps * FlagDirection;
		YOffset = Steps * (-FlagDirection);
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

	// If a piece should go forward (NORTH), 
	//	if it white, the X should be increased (X = 3; NewX = 4),
	//	otherwise, it should be decreased (X = 3; NewX = 2)
	// To do so, piece color value (casted to int) is used: 
	//	Black => -1
	//	White => +1
	XOffset = XOffset * static_cast<int>(PieceColor);
	YOffset = YOffset * static_cast<int>(PieceColor);

	return std::make_pair(XOffset, YOffset);
}









/* REPLAY (add to separated class) */
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
		TArray<FTileSaving> BoardToLoad = GameSaving[NumMove - 1];
		GField->LoadBoard(BoardToLoad);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("Bro cosa stai cercando")));
		GField->LoadBoard(CurrentBoard);
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
	UScrollBox* ScrollBox = Pawn->GetColor() == EPawnColor::WHITE ? 
		Cast<UScrollBox>(ReplayWidget->GetWidgetFromName(TEXT("scr_Replay_white")))
		: Cast<UScrollBox>(ReplayWidget->GetWidgetFromName(TEXT("scr_Replay_black")));
	if (World && ScrollBox && ButtonWidgetRef)
	{
		UUserWidget* WidgetBtn = CreateWidget(World, ButtonWidgetRef);
		if (WidgetBtn)
		{
			UTextBlock* BtnText = Cast<UTextBlock>(WidgetBtn->GetWidgetFromName(TEXT("txtBlock")));
			if (BtnText)
			{
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
	FString StartTileStr = TEXT("");
	for (auto& Piece : Tile->GetTileStatus().WhoCanGo)
	{
		// TODO => to test
		if (Pawn->GetType() == Piece->GetType() 
			&& Pawn->GetColor() == Piece->GetColor()
			&& Pawn->GetPieceNum() != Piece->GetPieceNum())
		{
			if (PreviousGridPosition[1] == Piece->GetGridPosition()[1])
			{
				StartTileStr += FString::FromInt(PreviousTile->GetNumberId());
			}
			else
			{
				StartTileStr += PreviousTile->GetLetterId().ToLower();
			}
		}
	}

	if (Pawn && !Tile->GetId().IsEmpty())
	{
		MoveStr = (IsPawn? TEXT("") : Pawn->GetId()) +
			StartTileStr +
			((IsPawn && EatFlag)? PreviousTile->GetLetterId().ToLower() : TEXT("")) +
			(EatFlag ? TEXT("x") : TEXT("")) + 
			Tile->GetId().ToLower() +
			((CheckFlag != EPawnColor::NONE &&  CheckMateFlag == EPawnColor::NONE)? TEXT("+") : TEXT("")) +
			(CheckMateFlag != EPawnColor::NONE ? TEXT("#") : TEXT(""));

		// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, MoveStr);
	}

	return MoveStr;
}