// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Players/Chess_PlayerInterface.h"
#include "GameField.h"
#include "GameFramework/GameModeBase.h"
#include "Chess_GameMode.generated.h"

class AActor;

struct FPosition;

/**
 * 
 */
UCLASS()
class CHESS_API AChess_GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	bool IsGameOver;
	TArray<IChess_PlayerInterface*> Players;
	int32 CurrentPlayer;
	int32 MoveCounter;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AGameField> GameFieldClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 FieldSize;

	UPROPERTY(VisibleAnywhere)
	AGameField* GField;

	// TODO
	// UPROPERTY(EditAnywhere, BlueprintReadOnly)
	// forse diversi pawn class

	AChess_GameMode();

	virtual void BeginPlay() override;

	void ChoosePlayerAndStartGame();

	void SetCellPawn(const int32 PlayerNumber, const FVector& SpawnPosition);

	int32 GetNextPlayer(int32 Player);

	void TurnNextPlayer();
};
