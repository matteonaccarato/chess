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
	Color = EColor::E;
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
	GameInstance->SetTurnMessage(TEXT("Human Turn"));
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
	
	

	//GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("CIAO MAMMA, turno mio? %d"), IsMyTurn));
	FHitResult Hit = FHitResult(ForceInit);
	GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursor(ECollisionChannel::ECC_Pawn, true, Hit);
	if (Hit.bBlockingHit && IsMyTurn)
	{
		// Select Pawn to move first
		// Then I must select the new position



		// TODO: controllare se la pedina è mia
		/* if (ATile* CurrTile = Cast<ATile>(Hit.GetActor()))
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("Selezionare pedina"));
		} */



		// TODO inizializzare sempre tutto (anche a nullptr)
		ABasePawn* PawnToEat = nullptr;
		AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());

		if (Cast<ABasePawn>(Hit.GetActor()))
		{
			// TODO => PawnTemp as variabile locale
			ABasePawn* PawnSelected = Cast<ABasePawn>(Hit.GetActor());
			if (PawnSelected && PawnSelected->GetColor() == EPawnColor::WHITE)
			{
				PawnTemp = PawnSelected;
				FVector2D CurrPawnGridPosition = PawnTemp->GetGridPosition();
				GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Blue, FString::Printf(TEXT("SELECTED X: %f Y: %f"), CurrPawnGridPosition[0], CurrPawnGridPosition[1]));
				GameMode->ShowPossibleMoves(PawnTemp, CurrPawnGridPosition[0], CurrPawnGridPosition[1]);
				SelectedPawnFlag = 1;
				
				
				// GameMode->GField->GetTileArray()[CurrPawnGridPosition[0]*8 + CurrPawnGridPosition[1]]->GetStaticMeshComponent()->SetMaterial(0, GameMode->GField->MaterialDark);
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

				if (GameMode->IsValidMove(PawnTemp, NewTile->GetGridPosition()[0], NewTile->GetGridPosition()[1], PawnToEat?true:false))
				{
					if (GameMode != nullptr)
					{
						/*
						* Elaborate new x,y in function of eligible moves of the pawn
						*
						* TODO: update TileArray and TileMap
						*/

						if (PawnToEat)
						{
							// TODO => c'è qualcosa da fare come destroy / deallocazione ?
							PawnToEat->SetStatus(EPawnStatus::DEAD);
							GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Blue, FString::Printf(TEXT("%f %f pawn has been eaten"), PawnToEat->GetGridPosition()[0], PawnToEat->GetGridPosition()[1]));
							// Hides visible components
							PawnToEat->SetActorHiddenInGame(true);

							// Disables collision components
							PawnToEat->SetActorEnableCollision(false);

							// Stops the Actor from ticking
							PawnToEat->SetActorTickEnabled(false);


						}


						NewTile->SetTileStatus(PlayerNumber, { 0, PawnTemp->GetColor(), PawnTemp->GetType() });
						NewTile->SetPawn(PawnTemp);
						GameMode->GField->GetTileArray()[PawnTemp->GetGridPosition()[0] * GameMode->GField->Size + PawnTemp->GetGridPosition()[1]]->SetPawn(nullptr);
						GameMode->GField->GetTileArray()[PawnTemp->GetGridPosition()[0] * GameMode->GField->Size + PawnTemp->GetGridPosition()[1]]->SetTileStatus(-1, { 1, EPawnColor::NONE, EPawnType::NONE });
						FVector SpawnPosition = NewTile->GetActorLocation() + FVector(0, 0, PawnTemp->GetActorLocation()[2]);
						PawnTemp->SetActorLocation(SpawnPosition);
						PawnTemp->SetGridPosition(NewTile->GetGridPosition()[0], NewTile->GetGridPosition()[1]);


						SelectedPawnFlag = 0;
						IsMyTurn = false;
						// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("SET TO FALSE")));

						GameMode->SetCellPawn(PlayerNumber, SpawnPosition);

					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("GameMode is null"));
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
		GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("%d ERROR"), IsMyTurn));
	}
}