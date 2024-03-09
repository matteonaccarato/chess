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
	GameInstance->SetTurnMessage(TEXT("Human"));

	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode != nullptr)
	{
		for (auto& Tile : GameMode->GField->GetTileArray())
		{
			FTileStatus TileStatus = Tile->GetTileStatus();
			TArray<bool> TmpFalse; 
			TmpFalse.SetNum(2, false);
			TileStatus.AttackableFrom = TmpFalse;
			TileStatus.WhoCanGo.Empty();
			// TileStatus.AttackableFrom = TArray<bool>(false, 2);
			Tile->SetTileStatus(TileStatus);
		}

		bool CanMove = false;
		for (auto& CurrPawn : GameMode->GField->GetPawnArray())
		{
			if (CurrPawn->GetColor() != GameMode->Players[GameMode->CurrentPlayer]->Color && CurrPawn->GetStatus() == EPawnStatus::ALIVE)
			{
				// TODO => fare tutte le show possible moves a priori nel turno, per evitare di calcolare ogni volta che clicco
				// ci accedo con pieceNum a array di array di possible moves
				TArray<std::pair<int8, int8>> Tmp = GameMode->ShowPossibleMoves(CurrPawn, true, true, true);
				if (Tmp.Num() > 0)
				{
					AttackableTiles.Add(Tmp);
					CanMove = true;
				}
			}
		}
		if (!CanMove)
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("HUMAN cannot move anything"));
			GameMode->EndTurn(-1);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode is null"));
	}

}

void AChess_HumanPlayer::OnWin()
{
	// TODO
	GameInstance->SetTurnMessage(TEXT("Human Wins!"));
	GameInstance->IncrementScoreHumanPlayer();
}

void AChess_HumanPlayer::OnLose()
{
	// TODO
	GameInstance->SetTurnMessage(TEXT("Human Loses!"));
}

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
			// clean last possible moves
			// vecchio material tramite pari/dispari della posizione
			// TODO => Itera su PossibleMoves per ripristinare il colore originario (serve saperlo o reperirlo in base a nome classe o stato della tile)

			for (const auto& move : PossibleMoves)
			{
				UMaterialInterface* Material = ((move.first + move.second) % 2)? GameMode->GField->MaterialLight : GameMode->GField->MaterialDark;
				GameMode->GField->GetTileArray()[move.first * GameMode->FieldSize + move.second]->GetStaticMeshComponent()->SetMaterial(0, Material);
			}


			if (Cast<ABasePawn>(Hit.GetActor()))
			{
				// TODO => PawnTemp as variabile locale
				ABasePawn* PawnSelected = Cast<ABasePawn>(Hit.GetActor());
				if (PawnSelected && PawnSelected->GetColor() == EPawnColor::WHITE)
				{
					PawnTemp = PawnSelected;
					PossibleMoves = GameMode->ShowPossibleMoves(PawnTemp);
					SelectedPawnFlag = 1;
				
				
				}
				else if (PawnSelected && PawnSelected->GetColor() == EPawnColor::BLACK && SelectedPawnFlag)
				{
					// Pawn to eat
					PawnToEat = PawnSelected;
					GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("PAWN TO EAT IS IN X: %f Y: %f FROM %f %f"), PawnToEat->GetGridPosition()[0], PawnToEat->GetGridPosition()[1], PawnTemp->GetGridPosition()[0], PawnTemp->GetGridPosition()[1]));
				}
			}

			if (SelectedPawnFlag == 1)
			{
				ATile* NewTile = Cast<ATile>(Hit.GetActor());
				if (PawnToEat)
				{
					NewTile = GameMode->GField->GetTileArray()[PawnToEat->GetGridPosition()[0] * GameMode->GField->Size + PawnToEat->GetGridPosition()[1]];
				}
				if (NewTile)
				{

					if (GameMode->IsValidMove(PawnTemp, NewTile->GetGridPosition()[0], NewTile->GetGridPosition()[1] /*, PawnToEat ? true : false*/))
					{
						
						/*
						* Elaborate new x,y in function of eligible moves of the pawn
						*/

						if (PawnToEat)
							GameMode->GField->DespawnPawn(PawnToEat->GetGridPosition()[0], PawnToEat->GetGridPosition()[1]);

						// vecchio material tramite pari/dispari della posizione
						// TODO => Itera su PossibleMoves per ripristinare il colore originario (serve saperlo o reperirlo in base a nome classe o stato della tile)

						NewTile->SetPlayerOwner(PlayerNumber);
						TArray<bool> TmpFalse; TmpFalse.Add(false); TmpFalse.Add(false);
						NewTile->SetTileStatus({ 0, TmpFalse, NewTile->GetTileStatus().WhoCanGo, PawnTemp->GetColor(), PawnTemp->GetType()});
						NewTile->SetPawn(PawnTemp);

						GameMode->GField->GetTileArray()[PawnTemp->GetGridPosition()[0] * GameMode->GField->Size + PawnTemp->GetGridPosition()[1]]->ClearInfo();
						
						PawnTemp->Move(NewTile);
						

						if (PawnTemp->GetType() == EPawnType::PAWN)
							PawnTemp->SetMaxNumberSteps(1);
						
						// GameMode->ShowPossibleMoves(PawnTemp, true, true, false);

						// update with last move
						GameMode->LastGridPosition = NewTile->GetGridPosition();
						GameMode->PreviousGridPosition = GameMode->GField->TileArray[PawnTemp->GetGridPosition()[0] * GameMode->GField->Size + PawnTemp->GetGridPosition()[1]]->GetGridPosition();
						SelectedPawnFlag = 0;
						IsMyTurn = false;

						
						// IF (...)
						// ELSE EndTurn
						if (NewTile->GetGridPosition()[0] == GameMode->GField->Size - 1 && PawnTemp->GetType() == EPawnType::PAWN)
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