// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM()
enum class EPawnType : uint8
{
	NONE,	// 0
	ROOK,	// 1
	KNIGHT, // 2
	BISHOP, // 3
	QUEEN,	// 4
	KING,	// 5
	PAWN,	// 6
};

/* 
 * Piece color (e.g. Black = -1, White = 1).
 *  Useful to identify the color and computing the movement direction easily:
 *   - White pieces move from the bottom to the top (+1 along the vertical axis),
 *	 - Black ones move from the top to the bottom (-1 along the vertical axis)
 */
 UENUM()
enum class EPawnColor : int8
{
	NONE = 0,	// No color assigned
	WHITE = 1,	// White color
	BLACK = -1,	// Black color
	BOTH = 2	// Both color to take into account both pieces of both the colors
};

UENUM()
enum class EMatchResult : int8
{
	NONE,					// nothing to notify
	WHITE,					// white checkmated
	BLACK,					// black checkmated
	STALEMATE,				// no eligible moves available but the king is not under check
	FIVEFOLD_REPETITION,	// five times the same board configuration
	SEVENTY_FIVE_MOVE_RULE,	// in the previous 75 moves by each side no pawns has move and no capture has been made
	INSUFFICIENT_MATERIAL	// impossibility to checkmate (e.g. king vs king, ...)
};

// TODO => ripetizione di quello sotto ?
UENUM()
enum class ECardinalDirection : uint8
{
	NORTH,
	NORTHEAST,
	EAST,
	SOUTHEAST,
	SOUTH,
	SOUTHWEST,
	WEST,
	NORTHWEST,
	KNIGHT_TR,	// top-right
	KNIGHT_TL,	// top-left
	KNIGHT_RT,	// right-top
	KNIGHT_RB,	// right-bottom
	KNIGHT_BR,	// bottom-right
	KNIGHT_BL,	// bottom-left
	KNIGHT_LB,	// left-bottom
	KNIGHT_LT	// left-top
};


// TODO => ripetizione di quello sopra ?
UENUM()
enum class EDirection : uint8
{
	FORWARD,
	BACKWARD,
	HORIZONTAL,
	DIAGONAL,
	KNIGHT
};

UENUM()
enum class ELine : uint8
{
	HORIZONTAL,
	VERTICAL,
	DIAGONAL
};

UENUM()
enum class EPawnStatus : int8
{
	ALIVE, // 0
	DEAD // 1
};

/**
 * 
 */
class CHESS_API ChessEnums
{
public:
	static const int32 NOT_ASSIGNED = -1;

	ChessEnums();
	~ChessEnums();
};
