// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/Rook.h"

ARook::ARook()
{
	Id = TEXT("R");
	Type = EPieceType::ROOK;
	MaxNumberSteps = ROOK_MAX_NUMBER_STEPS;
	CardinalDirections.Add(ECardinalDirection::NORTH);
	CardinalDirections.Add(ECardinalDirection::EAST);
	CardinalDirections.Add(ECardinalDirection::SOUTH);
	CardinalDirections.Add(ECardinalDirection::WEST);
}