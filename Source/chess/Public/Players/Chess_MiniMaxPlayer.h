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

	/* UTILS */
	static constexpr int8 DEPTH			    = 2; // One (excluded from this count) is built in FindBestMove function
	static constexpr int8 TIMER_MODULO		= 2;
	static constexpr int8 TIMER_BASE_OFFSET	= 1;

	/* EVALUATION | PIECES VALUES */
	static constexpr int  INFINITE			    = 10000;
	static constexpr int8 QUEEN_VALUE			= 9;
	static constexpr int8 ATTACKABLE_KING_VALUE = 8;
	static constexpr int8 ROOK_VALUE			= 5;
	static constexpr int8 BISHOP_VALUE			= 3;
	static constexpr int8 KNIGHT_VALUE			= 3;
	static constexpr int8 BLOCKING_KING_VALUE	= 2;
	static constexpr int8 PAWN_VALUE			= 1;


	/* METHODS */
	AChess_MiniMaxPlayer();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void OnTurn() override;
	virtual void OnWin() override;
	virtual void OnLose() override;

	std::pair<int8, std::pair<int8, int8>> FindBestMove(TArray<ATile*>& Board, TArray<std::pair<int8, TArray<std::pair<int8, int8>>>>& PlayerPieces);
	int32 MiniMax(TArray<ATile*>& Board, int8 Depth, int32 alpha, int32 beta, bool IsMax);
	int32 EvaluateBoard(TArray<ATile*> Board) const;
	int32 ComputeBlockingKingScore(const ABasePawn* KingToBlock) const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
