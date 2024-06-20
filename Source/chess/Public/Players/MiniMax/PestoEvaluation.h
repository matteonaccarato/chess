// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../../ChessEnums.h"

/**
 * 
 */
class CHESS_API PestoEvaluation
{
public:
	PestoEvaluation();
	~PestoEvaluation();

	/*
	 * Function: GetPieceSquareValue
	 * ----------------------------
	 * Get piece value based on its type, position and MiddleGame/EndGame
	 *
	 * @return			int32			Piece value
	 */
	static int32 GetPieceSquareValue(const EPieceType Type, int Position, const bool bIsEndGame) 
	{
		if (Position < 64)
		{
			switch (Type)
			{
			case EPieceType::PAWN:   return bIsEndGame ? EG_PawnTable[Position]   : MG_PawnTable[Position];
			case EPieceType::KNIGHT: return bIsEndGame ? EG_KnightTable[Position] : MG_KnightTable[Position];
			case EPieceType::BISHOP: return bIsEndGame ? EG_BishopTable[Position] : MG_BishopTable[Position];
			case EPieceType::ROOK:   return bIsEndGame ? EG_RookTable[Position]   : MG_RookTable[Position];
			case EPieceType::QUEEN:  return bIsEndGame ? EG_QueenTable[Position]  : MG_QueenTable[Position];
			case EPieceType::KING:   return bIsEndGame ? EG_KingTable[Position]   : MG_KingTable[Position];
			}
		}
		return 0;
	}


private:
	// Evaluation Tables based on Piece type and MiddleGame/EndGame situation
	static TArray<int> MG_PawnTable;
	static TArray<int> EG_PawnTable;

	static TArray<int> MG_KnightTable;
	static TArray<int> EG_KnightTable;

	static TArray<int> MG_BishopTable;
	static TArray<int> EG_BishopTable;

	static TArray<int> MG_RookTable;
	static TArray<int> EG_RookTable;

	static TArray<int> MG_QueenTable;
	static TArray<int> EG_QueenTable;

	static TArray<int> MG_KingTable;
	static TArray<int> EG_KingTable;
};
