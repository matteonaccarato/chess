// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Components/TextBlock.h"
#include "../BasePiece.h"
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
	/* ALGEBRAIC NOTATION */
	static constexpr char* SHORT_CASTLING = "0-0";
	static constexpr char* LONG_CASTLING  = "0-0-0";

	/* UI */
	static constexpr char* SCROLLBOX_WHITE_NAME = "scr_Replay_white";
	static constexpr char* SCROLLBOX_BLACK_NAME = "scr_Replay_black";
	static constexpr char* WIDGET_BUTTON_NAME	= "txtBlock";

	ReplayManager();
	~ReplayManager();


	/*
	 * Add the last move to the replay scrollbox.
	 * The piece is taken as argument, while the previous tile is taken from the attributes of the GameMode
	 *
	 * @param GameMode			const AChess_GameMode*	Gamemode to refer to
	 * @param Piece				const ABasePiece*		The piece which has been moved
	 * @param EatFlag			const bool = false		If another piece has been captured
	 * @param PawnPromotionFlag	const bool = false		If a pawn promotion has been happened
	 */
	static void AddToReplay(AChess_GameMode* GameMode, const ABasePiece* Piece, const bool EatFlag = false, const bool PawnPromotionFlag = false);
	

	/*
	 * Generate the algebraic notation of the last move.
	 * The piece is taken as argument, while the previous tile is taken from the attributes of the GameMode
	 *
	 * @param GameMode			AChess_GameMode*		GameMode to refer to
	 * @param Piece				const ABasePiece*		The piece which has been moved
	 * @param EatFlag			const bool = false		If another piece has been captured
	 * @param PawnPromotionFlag	const bool = false		If a pawn promotion has been happened
	 * 
	 * @return					FString					Move name
	 */
	static FString ComputeMoveName(const AChess_GameMode* GameMode, const ABasePiece* Piece, const bool EatFlag = false, const bool PawnPromotionFlag = false);
};
