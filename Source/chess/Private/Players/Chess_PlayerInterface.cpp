// Fill out your copyright notice in the Description page of Project Settings.


#include "Players/Chess_PlayerInterface.h"

// Add default functionality here for any IChess_PlayerInterface functions that are not pure virtual.

void IChess_PlayerInterface::OnDraw()
{
	if (GameInstance)
		GameInstance->SetTurnMessage(UChess_GameInstance::DRAW);
}
