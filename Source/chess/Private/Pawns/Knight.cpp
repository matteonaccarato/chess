// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/Knight.h"

AKnight::AKnight()
{
	Id = TEXT("N");
	Type = EPieceType::KNIGHT;
	MaxNumberSteps = KNIGHT_MAX_NUMBER_STEPS; 
	CardinalDirections.Add(ECardinalDirection::KNIGHT_TL);
	CardinalDirections.Add(ECardinalDirection::KNIGHT_TR);
	CardinalDirections.Add(ECardinalDirection::KNIGHT_RT);
	CardinalDirections.Add(ECardinalDirection::KNIGHT_RB);
	CardinalDirections.Add(ECardinalDirection::KNIGHT_BR);
	CardinalDirections.Add(ECardinalDirection::KNIGHT_BL);
	CardinalDirections.Add(ECardinalDirection::KNIGHT_LB);
	CardinalDirections.Add(ECardinalDirection::KNIGHT_LT);
}