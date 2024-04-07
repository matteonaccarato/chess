// Fill out your copyright notice in the Description page of Project Settings.

#include "Players/Chess_HumanPlayer.h"
#include "GameField.h"
#include "Chess_GameMode.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"


// Sets default values
AChess_HumanPlayer::AChess_HumanPlayer()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// Create camera component
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	// Set the camera as RootComponent
	SetRootComponent(Camera);

	// Get the game instance reference
	GameInstance = Cast<UChess_GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	// init values
	PlayerNumber = -1;
	Color = EPawnColor::WHITE;
}

// Called when the game starts or when spawned
void AChess_HumanPlayer::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
/* void AChess_HumanPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
} */

// Called to bind functionality to input
void AChess_HumanPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AChess_HumanPlayer::OnTurn()
{
	// IsMyTurn = true;
	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Green, TEXT("My Turn"));
	if (GameInstance)
	{
		GameInstance->SetTurnMessage(TEXT("Human"));
	}
}

void AChess_HumanPlayer::OnWin()
{
	// TODO
	if (GameInstance)
	{
		GameInstance->SetTurnMessage(TEXT("Human Wins!"));
		GameInstance->IncrementScoreHumanPlayer();
	}
}

void AChess_HumanPlayer::OnLose()
{
	// TODO
	if (GameInstance)
	{
		GameInstance->SetTurnMessage(TEXT("Human Loses!"));
	}
}

/*
 * Function: OnClick
 * ----------------------------
 *   Handling clicks made by the user:
 *	  - PawnTemp		: ABasePawn*	=>	auxiliary variable used to store the chosen piece to move
 *	  - PawnToEat		: ABasePawn*	=>	auxiliary variable used to store the piece to eat (if necessary)
 *	  - PawnSelected	: ABasePawn*	=>	Temporary variable to store the selected piece 
 *												(it can be the one to move or the one to eat).
 *											This decision is based on SelectedPawnFlag (bool)
 */
void AChess_HumanPlayer::OnClick()
{
	FHitResult Hit = FHitResult(ForceInit);
	GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursor(ECollisionChannel::ECC_Pawn, true, Hit);
	if (Hit.bBlockingHit && IsMyTurn)
	{
		// Select Pawn to move first
		// Then I must select the new position

		// TODO inizializzare sempre tutto (anche a nullptr)
		ABasePawn* PawnToEat = nullptr;
		AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
		if (GameMode)
		{
			// Clean shown possible moves of the selected piece
			for (const auto& move : GameMode->ShownPossibleMoves)
			{
				UMaterialInterface* Material = ((move.first + move.second) % 2) ? GameMode->GField->MaterialLight : GameMode->GField->MaterialDark;
				GameMode->GField->GetTileArray()[move.first * GameMode->GField->Size + move.second]->GetStaticMeshComponent()->SetMaterial(0, Material);
			}
			
			// Click on a chess piece actor <ABasePawn>
			if (Cast<ABasePawn>(Hit.GetActor()))
			{

				if (GameMode->ReplayInProgress != 0)
				{
					if (GameMode->ReplayInProgress % 2 == 0 || GameMode->ReplayInProgress == GameMode->MoveCounter)
					{
						// rewid activated
						GameMode->MoveCounter = GameMode->ReplayInProgress + (GameMode->ReplayInProgress == GameMode->MoveCounter ? 0 : 1);
						GameMode->ReplayInProgress = 0;
						GameMode->TurnPossibleMoves.Empty();
						GameMode->WhitePiecesCanMove.Empty();
						GameMode->BlackPiecesCanMove.Empty();

						// restore data in array from board
						for (auto& Tile : GameMode->GField->TileArray)
						{
							FTileStatus TileStatus = Tile->GetTileStatus();
							TileStatus.EmptyFlag = 1;
							TileStatus.PawnColor = EPawnColor::NONE;
							TileStatus.PawnType = EPawnType::NONE;
							TileStatus.AttackableFrom[0] = 0; TileStatus.AttackableFrom[1] = 0;
							TileStatus.WhoCanGo.Empty();
							Tile->SetTileStatus(TileStatus);
							Tile->SetPawn(nullptr);
						}

						for (const auto& Piece : GameMode->GField->PawnArray)
						{
							if (Piece->GetStatus() == EPawnStatus::ALIVE && GameMode->GField->IsValidTile(Piece->GetGridPosition()[0], Piece->GetGridPosition()[1]))
							{
								ATile* Tile = GameMode->GField->TileArray[Piece->GetGridPosition()[0] * GameMode->GField->Size + Piece->GetGridPosition()[1]];
								FTileStatus TileStatus = Tile->GetTileStatus();
								TileStatus.EmptyFlag = 0;
								TileStatus.PawnColor = Piece->GetColor();
								TileStatus.PawnType = Piece->GetType();
								Tile->SetTileStatus(TileStatus);
								Tile->SetPawn(Piece);

								switch (Piece->GetType())
								{
								case EPawnType::PAWN:
									if ((Piece->GetColor() == EPawnColor::WHITE && Piece->GetGridPosition()[0] == 1)
										|| (Piece->GetColor() == EPawnColor::BLACK && Piece->GetGridPosition()[0] == 6))
									{
										Piece->SetMaxNumberSteps(2);
									}
									else
									{
										Piece->SetMaxNumberSteps(1);
									}
									break;

								case EPawnType::ROOK:
									// TODO => far con enum o array di conversione (sono magic numbers)
									// rook left white
									switch (Piece->GetPieceNum())
									{
									case 0:
										GameMode->CastlingInfoWhite.RooksMoved[0] = !(Piece->GetGridPosition() == FVector2D(0, 0));
										break;
									case 7:
										GameMode->CastlingInfoWhite.RooksMoved[1] = !(Piece->GetGridPosition() == FVector2D(0, 7));
										break;
									case 24:
										GameMode->CastlingInfoBlack.RooksMoved[0] = !(Piece->GetGridPosition() == FVector2D(7, 0));
										break;
									case 31:
										GameMode->CastlingInfoBlack.RooksMoved[1] = !(Piece->GetGridPosition() == FVector2D(7, 7));
										break;
									}
									break;
								case EPawnType::KING:
									if (Piece->GetColor() == EPawnColor::WHITE)
									{
										GameMode->CastlingInfoWhite.KingMoved = !(Piece->GetGridPosition() == FVector2D(0, 4));
									}
									else
									{
										GameMode->CastlingInfoBlack.KingMoved = !(Piece->GetGridPosition() == FVector2D(7, 4));
									}
									break;
								}
							}
						}

						
						// TODO => Mettere in funzione a parte ma occhio che non funzionava
						for (auto& Piece : GameMode->GField->PawnArray)
						{
							EPawnColor PreviousCheckFlag = GameMode->CheckFlag;
							TArray<std::pair<int8, int8>> Tmp = GameMode->ShowPossibleMoves(Piece, false, true, true);
							GameMode->TurnPossibleMoves.Add(Tmp);
							GameMode->CheckFlag = PreviousCheckFlag;
							if (Tmp.Num() > 0)
							{
								switch (Piece->GetColor())
								{
								case EPawnColor::WHITE: GameMode->WhitePiecesCanMove.Add(Piece->GetPieceNum()); break;
								case EPawnColor::BLACK: GameMode->BlackPiecesCanMove.Add(Piece->GetPieceNum()); break;
								}
							}
						}

						
						// delete following moves from replay history
						if (GameMode->ReplayWidget)
						{
							for (UScrollBox* ScrollBox : { 
								Cast<UScrollBox>(GameMode->ReplayWidget->GetWidgetFromName(TEXT("scr_Replay_white"))), 
								Cast<UScrollBox>(GameMode->ReplayWidget->GetWidgetFromName(TEXT("scr_Replay_black"))) })
							{
								if (ScrollBox)
								{
									int8 NumChildren = ScrollBox->GetChildrenCount();
									int8 NumToRemove = NumChildren -static_cast<int>(GameMode->MoveCounter / 2);
									for (int8 i = 0; i < NumToRemove; i++)
									{
										ScrollBox->RemoveChildAt(NumChildren - i - 1);
										GameMode->GameSaving.RemoveAt(GameMode->GameSaving.Num() - 1);
									}
								}
							}
						}
					 }
					else
					{
						GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("BRO, non puoi riprendere quando è il turno del bot | PLEASE SELECT YUOUR TURN"));
						return;
					}
				}



				// TODO => PawnTemp as variabile locale
				ABasePawn* PawnSelected = Cast<ABasePawn>(Hit.GetActor());
				if (PawnSelected && PawnSelected->GetColor() == EPawnColor::WHITE)
				{
					// Selected human's chess piece (piece to move)
					PawnTemp = PawnSelected;

					// Visually show possible tile to go on
					if (GameMode->TurnPossibleMoves.IsValidIndex(PawnTemp->GetPieceNum()))
					{ 
						// TODO => serve ShownPossibleMoves o si può raggruppare con TurnPossibleMoves
						GameMode->ShownPossibleMoves = GameMode->TurnPossibleMoves[PawnTemp->GetPieceNum()];
						for (const auto& move : GameMode->ShownPossibleMoves)
						{
							UMaterialInterface* Material = ((move.first + move.second) % 2) ? GameMode->GField->MaterialLightRed : GameMode->GField->MaterialDarkRed;
							GameMode->GField->TileArray[move.first * GameMode->GField->Size + move.second]->GetStaticMeshComponent()->SetMaterial(0, Material);
						}
						// Notify white chess piece has been seleceted
						SelectedPawnFlag = 1;
					}
				}
				else if (PawnSelected && PawnSelected->GetColor() == EPawnColor::BLACK && SelectedPawnFlag)
				{
					// Black chess piece has been selected after a white one was already selected.
					// This black piece is the one to eat
					PawnToEat = PawnSelected;
				}
			}

			// Chess piece was already selected
			if (SelectedPawnFlag == 1)
			{
				// Get new tile instance (through PawnToEat OR actor clicked)
				ATile* NewTile = Cast<ATile>(Hit.GetActor());
				if (PawnToEat)
				{
					NewTile = GameMode->GField->TileArray[PawnToEat->GetGridPosition()[0] * GameMode->GField->Size + PawnToEat->GetGridPosition()[1]];
				}

				if (NewTile)
				{
					// Get PieceToEat if not properly selected by mouse clicking (e.g. the user clicked the tile and not the opponent chess piece)
					PawnToEat = NewTile->GetPawn();

					// TODO => già calcolato in turn possible moves
					// TurnPossibleMoves[PawnTemp->GetPieceNum()].Contains(x, y)
					// if (GameMode->IsValidMove(PawnTemp, NewTile->GetGridPosition()[0], NewTile->GetGridPosition()[1]))
					if (GameMode->TurnPossibleMoves[PawnTemp->GetPieceNum()].Contains(std::make_pair<int8,int8>(NewTile->GetGridPosition()[0], NewTile->GetGridPosition()[1])))
					{
						if (PawnToEat)
							GameMode->GField->DespawnPawn(PawnToEat->GetGridPosition()[0], PawnToEat->GetGridPosition()[1]);

						ATile* OldTile = GameMode->GField->TileArray[PawnTemp->GetGridPosition()[0] * GameMode->GField->Size + PawnTemp->GetGridPosition()[1]];
						PawnTemp->Move(OldTile, NewTile);

						// Castling Handling (King moves by two tiles)
						if (PawnTemp->GetType() == EPawnType::KING
							&& FMath::Abs(NewTile->GetGridPosition()[1] - OldTile->GetGridPosition()[1]) == 2)
						{
							// Move the rook
							bool ShortCastling = (NewTile->GetGridPosition()[1] - OldTile->GetGridPosition()[1]) > 0;
							int8 RookX = PawnTemp->GetColor() == EPawnColor::WHITE ? 0 : 7;
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
									GameMode->CastlingInfoWhite.KingMoved = true;
									GameMode->CastlingInfoWhite.RooksMoved[NewRookY == 0 ? 0 : 1];
								}
							}
						}

						if (PawnTemp->GetType() == EPawnType::ROOK)
						{
							GameMode->CastlingInfoWhite.RooksMoved[OldTile->GetGridPosition()[1] == 0 ? 0 : 1] = true;
						}
						

						// TODO => forse già fatto ?
						if (PawnTemp->GetType() == EPawnType::PAWN)
							PawnTemp->SetMaxNumberSteps(1);
						
						
						// Update info with last move
						GameMode->LastPiece = PawnTemp;
						GameMode->LastGridPosition = NewTile->GetGridPosition();
						GameMode->PreviousGridPosition = OldTile->GetGridPosition();
						GameMode->LastEatFlag = PawnToEat ? true : false;
						SelectedPawnFlag = 0;
						// IsMyTurn = false;


						// Pawn promotion handling
						if (NewTile->GetGridPosition()[0] == GameMode->GField->Size - 1 
							&& PawnTemp->GetType() == EPawnType::PAWN)
						{
							UWorld* World = GetWorld();							
							if (World && GameMode->PawnPromotionMenuWidgetRef)
							{
								GameMode->PawnPromotionWidget = CreateWidget<UUserWidget>(World, GameMode->PawnPromotionMenuWidgetRef, FName("Name"));
								GameMode->PawnPromotionWidget->AddToViewport(0);
							}
							else
							{
								UE_LOG(LogTemp, Error, TEXT("PawnPromotionMenu is null"));
							}
						}
						else
						{
							// GameMode->ComputeCheck();
							// TODO => superfluo, aggiunto in end turn
							GameMode->IsCheck();

							// GameMode->ShowPossibleMoves(PawnTemp, true, true, false);
							// GameMode->AddToReplay(PawnTemp, PawnToEat ? 1 : 0);
							GameMode->EndTurn(PlayerNumber);
						}						
					}
					else
					{
						GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("Invalid move"));
					}
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("GameMode is null"));
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("%d ERROR"), IsMyTurn));
	}
}