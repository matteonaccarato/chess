// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM()
enum class EPawnType : uint8
{
	NONE, // 0
	ROOK, // 1
	KNIGHT, // 2
	BISHOP, // 3
	QUEEN, // 4
	KING, // 5
	PAWN, // 6
};

// Color (Black = -1, Whitee = 1) per cambiare direzione se vincolata, tipo pedoni
UENUM()
enum class EPawnColor : int8
{
	NONE = 0,
	WHITE = 1,
	BLACK = -1
};

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
	KNIGHT_TR, // top-right
	KNIGHT_TL, // top-left
	KNIGHT_RT, // right-top
	KNIGHT_RB, // right-bottom
	KNIGHT_BR, // bottom-right
	KNIGHT_BL, // bottom-left
	KNIGHT_LB, // left-bottom
	KNIGHT_LT // left-top
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
