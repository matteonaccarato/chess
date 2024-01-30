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





			// TODO: prendere questo offset da attributo pawn




			// FVector NewLocation = AGameField::GetRelativeLocationByXYPosition(CurrPawnPosition[0], CurrPawnPosition[1]);
			/*FVector Origin;
			FVector BoxExtent;
			CurrPawn->GetActorBounds(false, Origin, BoxExtent);

			FVector PawnLocation(Origin.GetComponentForAxis(EAxis::X), Origin.GetComponentForAxis(EAxis::Y), Origin.GetComponentForAxis(EAxis::Z) + 100);
			ABasePawn* BasePawnObj = GetWorld()->SpawnActor<ABasePawn>(CurrPawn->GetClass(), PawnLocation, FRotator(0, 90, 0));
			// BasePawnObj->SetTileId(FString::Printf(TEXT("%c%d"), IdChar, IdNum));
			
			
			BasePawnObj->SetGridPosition(CurrPawnPosition[0], CurrPawnPosition[1] + 2);
			if (BasePawnObj != nullptr)
			{
				// 0.8 da mettere come attributo
				BasePawnObj->SetActorScale3D(CurrPawn->GetActorScale3D());
				// BasePawnObj->SetGridPosition(x, y);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("ABasePawn Obj is null"));
			} */


			// set tile status
			FVector SpawnPosition = CurrPawn->GetActorLocation();
			AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
			GameMode->SetCellPawn(PlayerNumber, SpawnPosition); // TODO: no fvector ma spawnposition










		}

		

		IsMyTurn = false;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("%d ERROR"), IsMyTurn));
		IsMyTurn = true;
	}
}

