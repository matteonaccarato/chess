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



		

		if (Cast<ABasePawn>(Hit.GetActor()))
		{
			PawnTemp = Cast<ABasePawn>(Hit.GetActor());
			if (PawnTemp && PawnTemp->GetColor() == EPawnColor::WHITE)
			{
				FVector2D CurrPawnGridPosition = PawnTemp->GetGridPosition();
				GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Blue, FString::Printf(TEXT("SELECTED X: %f Y: %f"), CurrPawnGridPosition[0], CurrPawnGridPosition[1]));
				SelectedPawnFlag = 1;
			}
		}




		if (SelectedPawnFlag == 1)
		{
			if (ATile* NewTile = Cast<ATile>(Hit.GetActor()))
			{
				
				AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
				if (GameMode->IsValidMove(PawnTemp, NewTile->GetGridPosition()[0], NewTile->GetGridPosition()[1]))
				{



					if (GameMode != nullptr)
					{
						/*
						* Elaborate new x,y in function of eligible moves of the pawn
						*
						* TODO: update TileArray and TileMap
						*/



						NewTile->SetTileStatus(PlayerNumber, { 0, PawnTemp->GetColor(), PawnTemp->GetType() });
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