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
	PrimaryActorTick.bCanEverTick = true;

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
void AChess_HumanPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AChess_HumanPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AChess_HumanPlayer::OnTurn()
{
	IsMyTurn = true;
	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Blue, TEXT("My Turn"));
	// GameInstance->SetTurnMessage(TEXT("Human Turn"));
}

void AChess_HumanPlayer::OnWin()
{
	// TODO
}

void AChess_HumanPlayer::OnLose()
{
	// TODO
}

void AChess_HumanPlayer::OnClick()
{

	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("CIAO MAMMA, turno mio? %d"), IsMyTurn));
	FHitResult Hit = FHitResult(ForceInit);
	GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursor(ECollisionChannel::ECC_Pawn, true, Hit);
	if (Hit.bBlockingHit && IsMyTurn)
	{
		// Select Pawn to move first
		// Then I must select the new position



		// TODO: controllare se la pedina � mia
		/* if (ATile* CurrTile = Cast<ATile>(Hit.GetActor()))
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("Selezionare pedina"));
		} */



		

		if (SelectedPawnFlag == 0)
		{
			PawnTemp = Cast<ABasePawn>(Hit.GetActor());
			if (PawnTemp)
			{
				FVector2D CurrPawnGridPosition = PawnTemp->GetGridPosition();
				GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Green, FString::Printf(TEXT("SELECTED X: %f Y: %f"), CurrPawnGridPosition[0], CurrPawnGridPosition[1]));
				SelectedPawnFlag = 1;
			}
		}




		if (SelectedPawnFlag == 1)
		{
			if (ATile* NewTile = Cast<ATile>(Hit.GetActor()))
			{
				// FVector2D CurrPawnPosition = PawnTemp->GetGridPosition();
				FVector2D NewGridPosition = NewTile->GetGridPosition();
				FVector2D CurrPawnGridPosition = PawnTemp->GetGridPosition();
				// if (IsValidMove(PawnTemp, NewTile))


				// NewGridPosition , CurrPawnPosition
				bool ValidMove = true;
				switch (PawnTemp->GetMovement())
				{
				case EPawnMovement::FORWARD:
					EPawnsColors DirectionFlag = PawnTemp->GetColor();

					if ((NewGridPosition[0] - CurrPawnGridPosition[0]) * static_cast<double>(DirectionFlag) > PawnTemp->MaxGetNumberSteps())
					{
						ValidMove = false;
					}

					GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Green, FString::Printf(TEXT("DIRECTION FLAG: %d"), DirectionFlag));
					break;
				/*case EPawnMovement::BACKWARD: break;
				case EPawnMovement::LEFT: break;
				case EPawnMovement::RIGHT: break;
				case EPawnMovement::DIAGONAL: break;*/
				
				/*default:
					ValidMove = false; */
				}



				if (NewTile->GetTileStatus() == ETileStatus::EMPTY)
				{

					AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
					if (GameMode != nullptr)
					{
						/*
						* Elaborate new x,y in function of eligible moves of the pawn
						*
						* TODO: update TileArray and TileMap
						*/

						NewTile->SetTileStatus(PlayerNumber, PawnTemp->GetType());
						FVector SpawnPosition = NewTile->GetActorLocation() + FVector(0, 0, PawnTemp->GetActorLocation()[2]);
						PawnTemp->SetActorLocation(SpawnPosition);
						PawnTemp->SetGridPosition(NewGridPosition[0], NewGridPosition[1]);
						
						
						
						
						GameMode->SetCellPawn(PlayerNumber, SpawnPosition);

						SelectedPawnFlag = 0;
						IsMyTurn = false;
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("GameMode is null"));
					}
				}



			}
		}




	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("%d ERROR"), IsMyTurn));
		IsMyTurn = true;
	}
}