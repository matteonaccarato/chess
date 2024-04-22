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
	/* ALGEBRAIC NOTATION */
	static constexpr char* SHORT_CASTLING = "0-0";
	static constexpr char* LONG_CASTLING  = "0-0-0";

	/* UI */
	static constexpr char* SCROLLBOX_WHITE_NAME = "scr_Replay_white";
	static constexpr char* SCROLLBOX_BLACK_NAME = "scr_Replay_black";
	static constexpr char* WIDGET_BUTTON_NAME	= "txtBlock";

	ReplayManager();
	~ReplayManager();


	static void AddToReplay(AChess_GameMode* GameMode, const ABasePawn* Pawn, const bool EatFlag = false, const bool PawnPromotionFlag = false);
	

	static FString ComputeMoveName(AChess_GameMode* GameMode, const ABasePawn* Pawn, const bool EatFlag = false, const bool PawnPromotionFlag = false);
};
