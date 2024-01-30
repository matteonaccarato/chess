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

void AChess_HumanPlayer::OnClick(EClickFlag flag)
{

	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Green, FString::Printf(TEXT("%d"), flag));


	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("CIAO MAMMA, turno mio? %d"), IsMyTurn));
	FHitResult Hit = FHitResult(ForceInit);
	GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursor(ECollisionChannel::ECC_Pawn, true, Hit);
	if (Hit.bBlockingHit && IsMyTurn)
	{
		// TODO: controllare se la pedina è mia
		if (ATile* CurrTile = Cast<ATile>(Hit.GetActor()))
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("Selezionare pedina"));
		}





		if (ABasePawn* CurrPawn = Cast<ABasePawn>(Hit.GetActor()))
		{
			
			// GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Green, CurrPawn->GetTileId());
			FVector2D CurrPawnPosition = CurrPawn->GetGridPosition();
			GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Green, FString::Printf(TEXT("%f %f"), CurrPawnPosition[0], CurrPawnPosition[1]));








			// Set up action bindings
			/* if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
			{
				EnhancedInputComponent->BindAction(ClickAction, ETriggerEvent::Triggered, this, &AChess_PlayerController::ClickOnGrid);
			} */








			// set tile status
			// FVector SpawnPosition = CurrPawn->GetActorLocation();
			AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
			if (GameMode != nullptr)
			{
				/*
				* Elaborate new x,y in function of eligible moves of the pawn
				* 
				* TODO: update TileArray and TileMap
				*/
				int32 NewX = CurrPawnPosition[0] + 2;
				int32 NewY = CurrPawnPosition[1];

				FVector SpawnPosition = GameMode->GField->GetRelativeLocationByXYPosition(NewX, NewY) + FVector(0,0,CurrPawn->GetActorLocation()[2]);
				CurrPawn->SetActorLocation(SpawnPosition);
				GameMode->SetCellPawn(PlayerNumber, SpawnPosition);
				
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("GameMode is null"));
			}
			

		}

		

		IsMyTurn = false;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("%d ERROR"), IsMyTurn));
		IsMyTurn = true;
	}
}

