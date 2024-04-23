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
	// Camera attached to the player pawn
	UCameraComponent* Camera;

	/* METHODS */
	AChess_HumanPlayer();

	// Called every frame
	// virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void OnTurn() override;
	virtual void OnWin() override;
	virtual void OnLose() override;

	/*
	 * Handle clicks made by the user (bound with left mouse click).
	 * Select the piece to move first, then selecting the new position is required (new tile or the piece to capture)
	 */
	UFUNCTION()
	void OnClick();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Auxiliary variable used to store the chosen piece to move
	ABasePawn* PawnTemp;

	// 0 => Select Pawn to move is needed
	// 1 => Pawn selected, Choose Tile to move on
	bool SelectedPawnFlag = false;
};
