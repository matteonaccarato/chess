// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "../Chess_GameInstance.h"
#include "MiniMax/PestoEvaluation.h"
#include "Chess_PlayerInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Chess_MiniMaxPlayer.generated.h"


UENUM()
enum class EEValuationFunction : uint8
{
	BASE,	// 0 - default
	PESTO,  // 1
};

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
	static constexpr int  INFINITE = 1000000;
	static constexpr int QUEEN_VALUE = 900;
	static constexpr int EG_QUEEN_VALUE = 900;
	static constexpr int ATTACKABLE_KING_VALUE = 800;
	static constexpr int PAWN_PROMOTION_BONUS = 800;
	static constexpr int ROOK_VALUE = 500;
	static constexpr int EG_ROOK_VALUE = 500;
	static constexpr int BISHOP_VALUE = 300;
	static constexpr int EG_BISHOP_VALUE = 300;
	static constexpr int KNIGHT_VALUE = 300;
	static constexpr int EG_KNIGHT_VALUE = 300;
	static constexpr int BLOCKING_KING_VALUE = 200;
	static constexpr int PAWN_VALUE = 100;
	static constexpr int EG_PAWN_VALUE = 100;

	
	/* EVALUATION | FUNCTION */
	EEValuationFunction EvaluationFunction;


	/* METHODS */
	AChess_MiniMaxPlayer();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void OnTurn() override;
	virtual void OnWin() override;
	virtual void OnLose() override;

	void SetEvaluationFunction(EEValuationFunction Evaluation);

	/* Returns piece value based on its type and MiddleGame/EngGame */ 
	int32 Type2Value(const EPieceType Type, const bool bIsEndgame) const;


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
	 * Evaluate the current board situation based on EvaluationFunction attribute:
	 *
	 * @return			int32			Board evaluation
	 */
	int32 EvaluateBoard() const;
	int32 Base() const;  // Base Evaluation
	int32 Pesto() const; // Advanced Evaluation (PeSTO)


	/*
	 * Compute if the king is blocked by opponent moves in some directions.
	 * "+" : represents the cell to check if they are attackable from opponent's pieces
	 *
	 *	+ + +
	 * 	+ K +
	 *	+ + +
	 *
	 * @param KingToBlock	const ABasePiece*	Pointer to the king to block the moves to
	 *
	 * @return				int32				Number of blocked directions  
	 */
	int32 ComputeBlockingKingScore(const ABasePiece* KingToBlock) const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
