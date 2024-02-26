// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/Knight.h"

AKnight::AKnight()
{
	Type = EPawnType::KNIGHT;
	MaxNumberSteps = 1; // TODO magic number AND maybe useless
	/* CardinalDirections.Add(ECardinalDirection::NORTHEAST);
	CardinalDirections.Add(ECardinalDirection::SOUTHEAST);
	CardinalDirections.Add(ECardinalDirection::SOUTHWEST);
	CardinalDirections.Add(ECardinalDirection::NORTHWEST); */ 
	// CardinalDirections.Add(ECardinalDirection::KNIGHT);
	CardinalDirections.Add(ECardinalDirection::KNIGHT_TL);
	CardinalDirections.Add(ECardinalDirection::KNIGHT_TR);
	CardinalDirections.Add(ECardinalDirection::KNIGHT_RT);
	CardinalDirections.Add(ECardinalDirection::KNIGHT_RB);
	CardinalDirections.Add(ECardinalDirection::KNIGHT_BR);
	CardinalDirections.Add(ECardinalDirection::KNIGHT_BL);
	CardinalDirections.Add(ECardinalDirection::KNIGHT_LB);
	CardinalDirections.Add(ECardinalDirection::KNIGHT_LT);
}