// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Components/TextBlock.h"
#include "../BasePawn.h"
#include "../Chess_GameMode.h"
#include "../GameField.h"
#include "../ChessEnums.h"
class AChess_GameMode;
struct FPieceSaving;

/**
 * Manage the creation of replay buttons and their add to scrollboxes (white / black)
 */
class CHESS_API ReplayManager
{
public:
	ReplayManager();
	~ReplayManager();

	/*
	 * Add the last move to the replay scrollbox.
 	 * The piece is taken as argument, while the previous tile is taken from the attributes of the GameMode
	 * 
	 * Pawn					const ABasePawn*		The pawn which has been moved
	 * EatFlag				const bool = false		If another piece has been captured
	 * PawnPromotionFlag	const bool = false		If a pawn promotion has been happened
 	 */
	static void AddToReplay(AChess_GameMode* GameMode, const ABasePawn* Pawn, const bool EatFlag = false, const bool PawnPromotionFlag = false);
	
	
	/* 
	 * Generate the algebraic notation of the last move.
 	 * The piece is taken as argument, while the previous tile is taken from the attributes of the GameMode
 	 *  (PreviousGridPosition: FVector2D)
	 * 
	 *  Pawn				const ABasePawn*		The pawn which has been moved
	 *	EatFlag				const bool = false		If another piece has been captured
	 *	PawnPromotionFlag	const bool = false		If a pawn promotion has been happened
	 */
	static FString ComputeMoveName(AChess_GameMode* GameMode, const ABasePawn* Pawn, const bool EatFlag = false, const bool PawnPromotionFlag = false);
};
