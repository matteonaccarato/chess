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
	IsMyTurn = true;
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
		if (GameMode && GameMode->CanPlay)
		{
			// Clean shown possible moves of the selected piece
			for (const auto& move : PossibleMoves)
			{
				UMaterialInterface* Material = ((move.first + move.second) % 2) ? GameMode->GField->MaterialLight : GameMode->GField->MaterialDark;
				GameMode->GField->GetTileArray()[move.first * GameMode->FieldSize + move.second]->GetStaticMeshComponent()->SetMaterial(0, Material);
			}
			
			// Click on a chess piece actor <ABasePawn>
			if (Cast<ABasePawn>(Hit.GetActor()))
			{
				// TODO => PawnTemp as variabile locale
				ABasePawn* PawnSelected = Cast<ABasePawn>(Hit.GetActor());
				if (PawnSelected && PawnSelected->GetColor() == EPawnColor::WHITE)
				{
					// Selected human's chess piece (piece to move)
					PawnTemp = PawnSelected;

					// Visually show possible tile to go on
					if (GameMode->TurnPossibleMoves.IsValidIndex(PawnTemp->GetPieceNum()))
					{ 
						PossibleMoves = GameMode->TurnPossibleMoves[PawnTemp->GetPieceNum()];
						for (const auto& move : PossibleMoves)
						{
							UMaterialInterface* Material = ((move.first + move.second) % 2) ? GameMode->GField->MaterialLightRed : GameMode->GField->MaterialDarkRed;
							GameMode->GField->GetTileArray()[move.first * GameMode->FieldSize + move.second]->GetStaticMeshComponent()->SetMaterial(0, Material);
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
					NewTile = GameMode->GField->GetTileArray()[PawnToEat->GetGridPosition()[0] * GameMode->GField->Size + PawnToEat->GetGridPosition()[1]];
				}

				if (NewTile)
				{
					// Get PieceToEat if not properly selected by mouse clicking (e.g. the user clicked the tile and not the opponent chess piece)
					PawnToEat = NewTile->GetPawn();

					// TODO => già calcolato in turn possible moves
					// TurnPossibleMoves[PawnTemp->GetPieceNum()].Contains(x, y)
					if (GameMode->IsValidMove(PawnTemp, NewTile->GetGridPosition()[0], NewTile->GetGridPosition()[1]))
					{
						if (PawnToEat)
							GameMode->GField->DespawnPawn(PawnToEat->GetGridPosition()[0], PawnToEat->GetGridPosition()[1]);

						ATile* OldTile = GameMode->GField->TileArray[PawnTemp->GetGridPosition()[0] * GameMode->GField->Size + PawnTemp->GetGridPosition()[1]];
						PawnTemp->Move(OldTile, NewTile);
						

						// TODO => forse già fatto ?
						if (PawnTemp->GetType() == EPawnType::PAWN)
							PawnTemp->SetMaxNumberSteps(1);
						
						
						// Update info with last move
						GameMode->LastGridPosition = NewTile->GetGridPosition();
						GameMode->PreviousGridPosition = OldTile->GetGridPosition();
						SelectedPawnFlag = 0;
						IsMyTurn = false;


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
							GameMode->AddToReplay(PawnTemp, PawnToEat? 1:0);
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