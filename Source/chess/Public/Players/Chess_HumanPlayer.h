// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/* UI */
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"

#include "CoreMinimal.h"
// #include "../Replay_GameMode.h"
#include "GameFramework/Pawn.h"
#include "../BasePawn.h"
#include "../Chess_GameInstance.h"
#include "Chess_PlayerInterface.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Chess_HumanPlayer.generated.h"


UCLASS()
class CHESS_API AChess_HumanPlayer : public APawn, public IChess_PlayerInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AChess_HumanPlayer();

	// Camera attached to the player pawn
	UCameraComponent* Camera;

	UChess_GameInstance* GameInstance;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool IsMyTurn = false;
	ABasePawn* PawnTemp;

	// 0 => Select Pawn to move is needed
	// 1 => Pawn selected, Choose Tile to move on
	// TODO: valutare se sostituibile con IsMyTurn
	int32 SelectedPawnFlag = 0;
	TArray<std::pair<int8, int8>> PossibleMoves;

public:	
	// Called every frame
	// virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void OnTurn() override;
	virtual void OnWin() override;
	virtual void OnLose() override;

	// called on left mouse click (binding)
	UFUNCTION()
	void OnClick();
};
