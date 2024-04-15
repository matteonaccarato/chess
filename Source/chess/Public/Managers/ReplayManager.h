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
 * 
 */
class CHESS_API ReplayManager
{
public:
	ReplayManager();
	~ReplayManager();

	static void AddToReplay(AChess_GameMode* GameMode, const ABasePawn* Pawn, const bool EatFlag = false, const bool PawnPromotionFlag = false);
	static FString ComputeMoveName(AChess_GameMode* GameMode, const ABasePawn* Pawn, const bool EatFlag = false, const bool PawnPromotionFlag = false);
};
