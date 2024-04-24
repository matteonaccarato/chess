// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/My_Pawn.h"

AMy_Pawn::AMy_Pawn()
{
	Id = TEXT("P");
	Type = EPieceType::PAWN;
	MaxNumberSteps = PAWN_MAX_NUMBER_STEPS;
	CardinalDirections.Add(ECardinalDirection::NORTH);
	CardinalDirections.Add(ECardinalDirection::NORTHEAST);
	CardinalDirections.Add(ECardinalDirection::NORTHWEST);
}
