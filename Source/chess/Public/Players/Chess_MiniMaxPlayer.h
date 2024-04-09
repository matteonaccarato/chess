// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "../Chess_GameInstance.h"
#include "Chess_PlayerInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Chess_MiniMaxPlayer.generated.h"

UCLASS()
class CHESS_API AChess_MiniMaxPlayer : public APawn, public IChess_PlayerInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AChess_MiniMaxPlayer();

	UChess_GameInstance* GameInstance;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void OnTurn() override;
	virtual void OnWin() override;
	virtual void OnLose() override;
	virtual void OnDraw() override;

	int32 EvaluateBoard(TArray<ATile*> Board) const;
	// are moves left (se ci sono mosse rimanenti => guardo PossibleMoves.Num()
	std::pair<std::pair<int8, std::pair<int8, int8>>, int32> MiniMax(TArray<ATile*> Board, int8 Depth, int32 alpha, int32 beta, bool IsMax);
	std::pair<int8, std::pair<int8, int8>> FindBestMove(TArray<ATile*> Board);
};
