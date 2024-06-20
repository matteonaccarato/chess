// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "../Chess_GameInstance.h"
#include "Chess_PlayerInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Chess_MiniMaxPlayer.generated.h"


UENUM()
enum class EEValuationFunction : uint8
{
	BASE,							// 0 - default
	SIMPLIFIED_PIECE_SQUARE_TABLES,	// 1
	PESTO,							// 2
};

UENUM()
enum class EGamePhase : uint8
{
	OPENING,	// 0
	MIDDLEGAME,	// 1
	ENDGAME		// 2
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
	static constexpr int  INFINITE			    = 1000000;
	/* static constexpr int8 QUEEN_VALUE = 9;
	static constexpr int8 ATTACKABLE_KING_VALUE = 8;
	static constexpr int8 ROOK_VALUE			= 5;
	static constexpr int8 BISHOP_VALUE			= 3;
	static constexpr int8 KNIGHT_VALUE			= 3;
	static constexpr int8 BLOCKING_KING_VALUE	= 2;
	static constexpr int8 PAWN_VALUE			= 1; */

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

	// pawn , knight , bishop , rook , queen , king
	const TArray<int>& OpeningMaterialWeights = { 82, 337, 365, 477, 1025,  0 };
	const TArray<int>& EndgameMaterialWeights = { 94, 281, 297, 512,  936,  0 };


	int GetGamePhaseScore() const;

	bool IsEndGame(int8 QueenCounts[2], int8 RookCounts[2], int8 BishopCounts[2], int8 KnightsCounts[2]) const;

	// TABLES from White's point of view (at the bottom and maximizing the board score)
	const TArray<int>& MG_PawnTable = {
		 0,   0,   0,   0,   0,   0,  0,   0,
		 98, 134,  61,  95,  68, 126, 34, -11,
		 -6,   7,  26,  31,  65,  56, 25, -20,
		-14,  13,   6,  21,  23,  12, 17, -23,
		-27,  -2,  -5,  12,  17,   6, 10, -25,
		-26,  -4,  -4, -10,   3,   3, 33, -12,
		-35,  -1, -20, -23, -15,  24, 38, -22,
		  0,   0,   0,   0,   0,   0,  0,   0,
	};

	const TArray<int>& MG_KnightTable = {
		-167, -89, -34, -49,  61, -97, -15, -107,
	 -73, -41,  72,  36,  23,  62,   7,  -17,
	 -47,  60,  37,  65,  84, 129,  73,   44,
	  -9,  17,  19,  53,  37,  69,  18,   22,
	 -13,   4,  16,  13,  28,  19,  21,   -8,
	 -23,  -9,  12,  10,  19,  17,  25,  -16,
	 -29, -53, -12,  -3,  -1,  18, -14,  -19,
	-105, -21, -58, -33, -17, -28, -19,  -23,
	};


	const TArray<int>& MG_BishopTable = {
		-29,   4, -82, -37, -25, -42,   7,  -8,
	-26,  16, -18, -13,  30,  59,  18, -47,
	-16,  37,  43,  40,  35,  50,  37,  -2,
	 -4,   5,  19,  50,  37,  37,   7,  -2,
	 -6,  13,  13,  26,  34,  12,  10,   4,
	  0,  15,  15,  15,  14,  27,  18,  10,
	  4,  15,  16,   0,   7,  21,  33,   1,
	-33,  -3, -14, -21, -13, -12, -39, -21,
	};

	const TArray<int>& MG_RookTable = {
		32,  42,  32,  51, 63,  9,  31,  43,
	 27,  32,  58,  62, 80, 67,  26,  44,
	 -5,  19,  26,  36, 17, 45,  61,  16,
	-24, -11,   7,  26, 24, 35,  -8, -20,
	-36, -26, -12,  -1,  9, -7,   6, -23,
	-45, -25, -16, -17,  3,  0,  -5, -33,
	-44, -16, -20,  -9, -1, 11,  -6, -71,
	-19, -13,   1,  17, 16,  7, -37, -26,
	};

	const TArray<int>& MG_QueenTable = {
		-28,   0,  29,  12,  59,  44,  43,  45,
	-24, -39,  -5,   1, -16,  57,  28,  54,
	-13, -17,   7,   8,  29,  56,  47,  57,
	-27, -27, -16, -16,  -1,  17,  -2,   1,
	 -9, -26,  -9, -10,  -2,  -4,   3,  -3,
	-14,   2, -11,  -2,  -5,   2,  14,   5,
	-35,  -8,  11,   2,   8,  15,  -3,   1,
	 -1, -18,  -9,  10, -15, -25, -31, -50,
	};

	const TArray<int>& MG_KingTable = {
		-65,  23,  16, -15, -56, -34,   2,  13,
	 29,  -1, -20,  -7,  -8,  -4, -38, -29,
	 -9,  24,   2, -16, -20,   6,  22, -22,
	-17, -20, -12, -27, -30, -25, -14, -36,
	-49,  -1, -27, -39, -46, -44, -33, -51,
	-14, -14, -22, -46, -44, -30, -15, -27,
	  1,   7,  -8, -64, -43, -16,   9,   8,
	-15,  36,  12, -54,   8, -28,  24,  14,
	};

	const TArray<int>& EG_PawnTable = {
		 0,   0,   0,   0,   0,   0,   0,   0,
	178, 173, 158, 134, 147, 132, 165, 187,
	 94, 100,  85,  67,  56,  53,  82,  84,
	 32,  24,  13,   5,  -2,   4,  17,  17,
	 13,   9,  -3,  -7,  -7,  -8,   3,  -1,
	  4,   7,  -6,   1,   0,  -5,  -1,  -8,
	 13,   8,   8,  10,  13,   0,   2,  -7,
	  0,   0,   0,   0,   0,   0,   0,   0,
	};

	const TArray<int>& EG_KnightTable = {
		-58, -38, -13, -28, -31, -27, -63, -99,
	-25,  -8, -25,  -2,  -9, -25, -24, -52,
	-24, -20,  10,   9,  -1,  -9, -19, -41,
	-17,   3,  22,  22,  22,  11,   8, -18,
	-18,  -6,  16,  25,  16,  17,   4, -18,
	-23,  -3,  -1,  15,  10,  -3, -20, -22,
	-42, -20, -10,  -5,  -2, -20, -23, -44,
	-29, -51, -23, -15, -22, -18, -50, -64,
	};


	const TArray<int>& EG_BishopTable = {
		 -14, -21, -11,  -8, -7,  -9, -17, -24,
	 -8,  -4,   7, -12, -3, -13,  -4, -14,
	  2,  -8,   0,  -1, -2,   6,   0,   4,
	 -3,   9,  12,   9, 14,  10,   3,   2,
	 -6,   3,  13,  19,  7,  10,  -3,  -9,
	-12,  -3,   8,  10, 13,   3,  -7, -15,
	-14, -18,  -7,  -1,  4,  -9, -15, -27,
	-23,  -9, -23,  -5, -9, -16,  -5, -17,
	};

	const TArray<int>& EG_RookTable = {
		13, 10, 18, 15, 12,  12,   8,   5,
	11, 13, 13, 11, -3,   3,   8,   3,
	 7,  7,  7,  5,  4,  -3,  -5,  -3,
	 4,  3, 13,  1,  2,   1,  -1,   2,
	 3,  5,  8,  4, -5,  -6,  -8, -11,
	-4,  0, -5, -1, -7, -12,  -8, -16,
	-6, -6,  0,  2, -9,  -9, -11,  -3,
	-9,  2,  3, -1, -5, -13,   4, -20,
	};

	const TArray<int>& EG_QueenTable = {
		 -9,  22,  22,  27,  27,  19,  10,  20,
	-17,  20,  32,  41,  58,  25,  30,   0,
	-20,   6,   9,  49,  47,  35,  19,   9,
	  3,  22,  24,  45,  57,  40,  57,  36,
	-18,  28,  19,  47,  31,  34,  39,  23,
	-16, -27,  15,   6,   9,  17,  10,   5,
	-22, -23, -30, -16, -16, -23, -36, -32,
	-33, -28, -22, -43,  -5, -32, -20, -41,
	};

	const TArray<int>& EG_KingTable = {
		-74, -35, -18, -18, -11,  15,   4, -17,
	-12,  17,  14,  17,  17,  38,  23,  11,
	 10,  17,  23,  15,  20,  45,  44,  13,
	 -8,  22,  24,  27,  26,  33,  26,   3,
	-18,  -4,  21,  24,  27,  23,   9, -11,
	-19,  -3,  11,  21,  23,  16,   7,  -9,
	-27, -11,   4,  13,  14,   4,  -5, -17,
	-53, -34, -21, -11, -28, -14, -24, -43
	};
































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


	int32 Base() const;
	int32 Pesto() const;

	int32 GetPieceValueByMaterial(const ABasePiece* Piece, const EGamePhase GamePhase) const;
	int32 Type2Value(const EPieceType Type, const bool bIsEndgame) const;
	int32 GetPieceSquareValue(const EPieceType Type, const EPieceColor PieceColor, int X, const int Y, const bool bIsEndGame) const;





	// TODO => FROM evaluation enum TO pointer to function
	// constructor minimax(enum2function(PESTO)) => parametro assegnato a attributo di minimax player. l'attributo EvaluateBoard() è un puntatore a funzione










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
	int32 EvaluateBoard() const;


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
