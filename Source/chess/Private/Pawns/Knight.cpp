// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/Knight.h"

AKnight::AKnight()
{
	Type = EPawnType::KNIGHT;
	MaxNumberSteps = 1; // TODO magic number AND maybe useless
	/* CardinalDirections.push_back(ECardinalDirection::NORTHEAST);
	CardinalDirections.push_back(ECardinalDirection::SOUTHEAST);
	CardinalDirections.push_back(ECardinalDirection::SOUTHWEST);
	CardinalDirections.push_back(ECardinalDirection::NORTHWEST); */ 
	// CardinalDirections.push_back(ECardinalDirection::KNIGHT);
	CardinalDirections.push_back(ECardinalDirection::KNIGHT_TL);
	CardinalDirections.push_back(ECardinalDirection::KNIGHT_TR);
	CardinalDirections.push_back(ECardinalDirection::KNIGHT_RT);
	CardinalDirections.push_back(ECardinalDirection::KNIGHT_RB);
	CardinalDirections.push_back(ECardinalDirection::KNIGHT_BR);
	CardinalDirections.push_back(ECardinalDirection::KNIGHT_BL);
	CardinalDirections.push_back(ECardinalDirection::KNIGHT_LB);
	CardinalDirections.push_back(ECardinalDirection::KNIGHT_LT);
}