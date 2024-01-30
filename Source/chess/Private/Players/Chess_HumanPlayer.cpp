// Fill out your copyright notice in the Description page of Project Settings.


#include "Players/Chess_HumanPlayer.h"
#include "GameField.h"
#include "Chess_GameMode.h"
/*
* #include "Components/InputComponent.h"
* #include "EnhancedInputComponent.h"
* #include "EnhancedInputSubsystems.h"
*/

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
}

void AChess_HumanPlayer::OnWin()
{
}

void AChess_HumanPlayer::OnLose()
{
}

void AChess_HumanPlayer::OnClick()
{
}

