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

	// Initialise values
	PlayerNumber = -1;
	Color = EPieceColor::WHITE;
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
	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Green, UChess_GameInstance::HUMAN_TURN);
	if (GameInstance)
		GameInstance->SetTurnMessage(UChess_GameInstance::HUMAN_TURN);
}

void AChess_HumanPlayer::OnWin()
{
	if (GameInstance)
	{
		GameInstance->SetTurnMessage(UChess_GameInstance::HUMAN_WIN);

		PlayerNumber ?
			GameInstance->IncrementScorePlayer_2() :
			GameInstance->IncrementScorePlayer_1();
	}
}

void AChess_HumanPlayer::OnLose()
{
	if (GameInstance)
		GameInstance->SetTurnMessage(UChess_GameInstance::HUMAN_DEFEAT);
}

/*
 * Function: OnClick
 * ----------------------------
 * Handle clicks made by the user (bound with left mouse click).
 * Select the piece to move first, then selecting the new position is required (new tile or the piece to capture)
 */
void AChess_HumanPlayer::OnClick()
{
	FHitResult Hit = FHitResult(ForceInit);
	GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursor(ECollisionChannel::ECC_Pawn, true, Hit);
	if (Hit.bBlockingHit && IsMyTurn)
	{
		// Auxiliary variable used to store the piece to eat(if necessary)
		ABasePiece* PieceToEat = nullptr;
		AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
		if (GameMode)
		{
			// Clean shown possible moves of the selected piece
			for (const auto& move : GameMode->ShownPossibleMoves)
			{
				UMaterialInterface* Material = ((move.first + move.second) % 2) ? GameMode->GField->MaterialsLight[ETileMaterialType::STANDARD] : GameMode->GField->MaterialsDark[ETileMaterialType::STANDARD];
				GameMode->GField->TileArray[move.first * GameMode->GField->Size + move.second]->GetStaticMeshComponent()->SetMaterial(0, Material);
			}
			
			// Click on a chess piece actor <ABasePiece>
			if (Cast<ABasePiece>(Hit.GetActor()))
			{

				if (GameMode->ReplayInProgress != 0)
				{
					// If a rewind is activated by the user.
					// It must be required on a human turn (so on even turn number, since the human is always the first one to begin)
					if (GameMode->ReplayInProgress % 2 == 0 || GameMode->ReplayInProgress == GameMode->MoveCounter)
					{
						// Rewid activated
						GameMode->MoveCounter = GameMode->ReplayInProgress + (GameMode->ReplayInProgress == GameMode->MoveCounter ? 0 : 1);
						GameMode->ReplayInProgress = 0;
						GameMode->TurnPossibleMoves.Empty();
						GameMode->WhitePiecesCanMove.Empty();
						GameMode->BlackPiecesCanMove.Empty();

						// Restore tiles data in array from board
						for (auto& Tile : GameMode->GField->TileArray)
						{
							FTileStatus TileStatus = Tile->GetTileStatus();
							TileStatus.EmptyFlag = 1;
							TileStatus.PieceColor = EPieceColor::NONE;
							TileStatus.PieceType = EPieceType::NONE;
							TileStatus.AttackableFrom[0] = 0; TileStatus.AttackableFrom[1] = 0;
							TileStatus.WhoCanGo.Empty();
							Tile->SetTileStatus({ 
								nullptr, 
								1, 
								{ 0, 0 },
								Tile->GetTileStatus().WhoCanGo,
								EPieceColor::NONE,
								EPieceType::NONE 
							});
						} 

						// Restore pieces data in array from board
						for (const auto& Piece : GameMode->GField->PieceArray)
						{
							if (Piece->GetStatus() == EPieceStatus::ALIVE && GameMode->GField->IsValidTile(Piece->GetGridPosition()[0], Piece->GetGridPosition()[1]))
							{
								ATile* Tile = GameMode->GField->TileArray[Piece->GetGridPosition()[0] * GameMode->GField->Size + Piece->GetGridPosition()[1]];
								Tile->SetTileStatus({
									Piece,
									0,
									{ 0, 0 },
									{},
									Piece->GetColor(),
									Piece->GetType()
								});

								if (Piece->GetType() == EPieceType::PAWN)
								{
									if ((Piece->GetColor() == EPieceColor::WHITE && Piece->GetGridPosition()[0] == 1)
										|| (Piece->GetColor() == EPieceColor::BLACK && Piece->GetGridPosition()[0] == GameMode->GField->Size - 2))
									{
										Piece->SetMaxNumberSteps(2);
									}
									else
									{
										Piece->SetMaxNumberSteps(1);
									}
								}
							}
						}

						// Compute check situation
						GameMode->IsCheck();

						if (GameMode->CheckFlag != EPieceColor::NONE)
						{
							GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Yellow, FString::Printf(TEXT("King under check | %d"), GameMode->CheckFlag));
							UGameplayStatics::PlaySound2D(GetWorld(), GameMode->CheckSound, 1, 1, 0, NULL, false, true);

							const ABasePiece* KingUnderCheck = GameMode->CheckFlag == EPieceColor::WHITE ?
								GameMode->GField->PieceArray[GameMode->KingWhitePieceNum] :
								GameMode->GField->PieceArray[GameMode->KingBlackPieceNum];
							int8 x = KingUnderCheck->GetGridPosition()[0];
							int8 y = KingUnderCheck->GetGridPosition()[1];
							UMaterialInterface* Material = ((x + y) % 2) ?
								GameMode->GField->MaterialsLight[ETileMaterialType::CHECK] :
								GameMode->GField->MaterialsDark[ETileMaterialType::CHECK];
							GameMode->GField->TileArray[x * GameMode->GField->Size + y]->GetStaticMeshComponent()->SetMaterial(0, Material);
							GameMode->GField->TileRestoreMaterialCoordinates = FVector2D(x, y);
						}
						
						// Compute all possible moves
						for (const auto& Piece : GameMode->GField->PieceArray)
						{
							TArray<std::pair<int8, int8>> Tmp = GameMode->ShowPossibleMoves(Piece, false, true, true);
							GameMode->TurnPossibleMoves.Add(Tmp);
							if (Tmp.Num() > 0)
							{
								switch (Piece->GetColor())
								{
								case EPieceColor::WHITE: GameMode->WhitePiecesCanMove.Add(std::make_pair(Piece->GetPieceNum(), Tmp)); break;
								case EPieceColor::BLACK: GameMode->BlackPiecesCanMove.Add(std::make_pair(Piece->GetPieceNum(), Tmp)); break;
								}
							}
						}

						// Delete following moves from replay history
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
						GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("Bot turn | PLEASE SELECT YUOUR TURN"));
						return;
					}
				}

				// Temporary variable to store the selected piece (it can be the one to move or the one to eat).
				// This decision is based on SelectedPawnFlag (bool)
				ABasePiece* PawnSelected = Cast<ABasePiece>(Hit.GetActor());
				if (PawnSelected && PawnSelected->GetColor() == EPieceColor::WHITE)
				{
					// Selected human's chess piece (piece to move)
					PieceTemp = PawnSelected;

					// Visually show possible tile to go on
					if (GameMode->TurnPossibleMoves.IsValidIndex(PieceTemp->GetPieceNum()))
					{
						GameMode->ShownPossibleMoves = GameMode->TurnPossibleMoves[PieceTemp->GetPieceNum()];
						for (const auto& move : GameMode->ShownPossibleMoves)
						{
							UMaterialInterface* Material = ((move.first + move.second) % 2) ? GameMode->GField->MaterialsLight[ETileMaterialType::ACTIVE] : GameMode->GField->MaterialsDark[ETileMaterialType::ACTIVE];
							GameMode->GField->TileArray[move.first * GameMode->GField->Size + move.second]->GetStaticMeshComponent()->SetMaterial(0, Material);
						}
						// Notify white chess piece has been seleceted
						SelectedPieceFlag = true;
					}
				}
				else if (PawnSelected && PawnSelected->GetColor() == EPieceColor::BLACK && SelectedPieceFlag)
				{
					// Black chess piece has been selected after a white one was already selected.
					// This black piece is the one to capture
					PieceToEat = PawnSelected;
				}
			}

			// Chess piece was already selected
			if (SelectedPieceFlag)
			{
				// Get new tile instance (through PawnToEat OR actor clicked)
				ATile* NewTile = Cast<ATile>(Hit.GetActor());
				if (PieceToEat)
					NewTile = GameMode->GField->TileArray[PieceToEat->GetGridPosition()[0] * GameMode->GField->Size + PieceToEat->GetGridPosition()[1]];
				
				if (NewTile)
				{
					// Get PieceToEat if not properly selected by mouse clicking (e.g. the user clicked the tile and not the actor of the opponent chess piece)
					PieceToEat = NewTile->GetPiece();

					if (GameMode->TurnPossibleMoves[PieceTemp->GetPieceNum()].Contains(std::make_pair<int8,int8>(NewTile->GetGridPosition()[0], NewTile->GetGridPosition()[1])))
					{
						bool EatFlag = GameMode->MakeMove(PieceTemp, NewTile->GetGridPosition()[0], NewTile->GetGridPosition()[1]);

						SelectedPieceFlag = false;
						
						// Pawn promotion handling
						int8 OpponentSide = Color == EPieceColor::WHITE ? GameMode->GField->Size - 1 : 0;
						if (NewTile->GetGridPosition()[0] == OpponentSide
							&& PieceTemp->GetType() == EPieceType::PAWN)
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
							// End Turn 
							GameMode->LastPiece = PieceTemp;
							GameMode->LastEatFlag = EatFlag;
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
}