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


	/*
	 * Analyse all the possible moves the player can make and choose the best one possible based on the current chess board situation.
	 * Its core stands in the MiniMax algorithm implemented in the MiniMax function with the integration of the alpha-beta pruning
	 *
	 * @param Board			TArray<ATile>*												Current board made of tiles
	 * @param PlayerPieces	TArray<std::pair<int8, TArray<std::pair<int8, int8>>>>&		Current player pieces which can move. 
	 *																					The structure of a single element of the outer TArray is <piece_number, TArray<new_x, new_y>>
	 * 
	 * @return				std::pair<int8, std::pair<int8, int8>>						The best move to make where the structure is 
	 *																					<piece_number, <new_x, new_y>>
	 */
	std::pair<int8, std::pair<int8, int8>> FindBestMove(TArray<ATile*>& Board, TArray<std::pair<int8, TArray<std::pair<int8, int8>>>>& PlayerPieces) const;
	
	
	/*
	 * Analyse all the possible moves the player can make and choose the best one possible based on the current chess board situation.
	 * The black player has to maximize the board evaluation, while the white one has to minimize it
	 *
	 * @param Board		TArray<ATile>*	Current board made of tiles
	 * @param Depth		int8			Depth of the minimax algorithm
	 * @param Alpha		int32			Store the alpha value to allow alpha-beta pruning implentation to improve time complexity
	 * @param Beta		int32			Store the beta value to allow alpha-beta pruning implentation to improve time complexity
	 * 
	 * @return			int32			Best board evaluation
	 */
	int32 MiniMax(TArray<ATile*>& Board, int8 Depth, int32 alpha, int32 beta, bool IsMax) const;


	/*
	 * Evaluate the current board situation through function f defined as follows:
	 * f = QUEEN_VALUE * (Q' - Q)
	 *		+ ATTACKABLE_KING_VALUE * (AK - AK')
	 *		+ BLOCKING_KING_VALUE * (BK' - BK)
	 *		+ ROOK_VALUE * (R' - R)
	 * 		+ BISHOP_VALUE * (B' - B)
	 *		+ KNIGHT_VALUE * (N' - N)
	 *		+ PAWN_VALUE * (P' - P)
	 * The prime value (e.g. X') means the number of pieces of type X of the black player,
	 * while the standard value (e.g. X) indicates the one of the white player
	 *
	 * @param Board		TArray<ATile>*	Current board made of tiles
	 * 
	 * @return			int32			Board evaluation
	 */
	int32 EvaluateBoard(TArray<ATile*> Board) const;


	/*
	 * Compute if the king is blocked by opponent moves in some directions.
	 * "+" : represents the cell to check if they are attackable from opponent's pieces
	 *
	 *	+ + +
	 * 	+ K +
	 *	+ + +
	 *
	 * @param KingToBlock	const ABasePawn*	Pointer to the king to block the moves to
	 *
	 * @return				int32				Number of blocked directions  
	 */
	int32 ComputeBlockingKingScore(const ABasePawn* KingToBlock) const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
