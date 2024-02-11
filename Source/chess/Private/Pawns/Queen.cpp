// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/Queen.h"

AQueen::AQueen()
{
	Type = EPawnType::QUEEN;
	MaxNumberSteps = 8; // TODO magic number
	CardinalDirections.push_back(ECardinalDirection::NORTH);
	CardinalDirections.push_back(ECardinalDirection::NORTHEAST);
	CardinalDirections.push_back(ECardinalDirection::EAST);
	CardinalDirections.push_back(ECardinalDirection::SOUTHEAST);
	CardinalDirections.push_back(ECardinalDirection::SOUTH);
	CardinalDirections.push_back(ECardinalDirection::SOUTHWEST);
	CardinalDirections.push_back(ECardinalDirection::WEST);
	CardinalDirections.push_back(ECardinalDirection::NORTHWEST);
}