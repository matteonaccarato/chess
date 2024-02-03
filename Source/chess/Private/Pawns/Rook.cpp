// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/Rook.h"

ARook::ARook()
{
	Type = EPawnType::ROOK;
	// Movement = EPawnMovement::FORWARD;
	/*AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode != nullptr)
	{ */
		MaxNumberSteps = 8; // TODO magic number
	/* }
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode is null"));
	}*/
}