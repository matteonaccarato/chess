// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/Queen.h"

AQueen::AQueen()
{
	Type = EPawnType::QUEEN;
	MaxNumberSteps = 8; // TODO magic number
	CardinalDirections.Add(ECardinalDirection::NORTH);
	CardinalDirections.Add(ECardinalDirection::NORTHEAST);
	CardinalDirections.Add(ECardinalDirection::EAST);
	CardinalDirections.Add(ECardinalDirection::SOUTHEAST);
	CardinalDirections.Add(ECardinalDirection::SOUTH);
	CardinalDirections.Add(ECardinalDirection::SOUTHWEST);
	CardinalDirections.Add(ECardinalDirection::WEST);
	CardinalDirections.Add(ECardinalDirection::NORTHWEST);
}