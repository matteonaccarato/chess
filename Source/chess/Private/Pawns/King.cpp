// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/King.h"

AKing::AKing()
{
	Type = EPawnType::KING;
	MaxNumberSteps = 1; // TODO magic number
	CardinalDirections.push_back(ECardinalDirection::NORTH);
	CardinalDirections.push_back(ECardinalDirection::NORTHEAST);
	CardinalDirections.push_back(ECardinalDirection::EAST);
	CardinalDirections.push_back(ECardinalDirection::SOUTHEAST);
	CardinalDirections.push_back(ECardinalDirection::SOUTH);
	CardinalDirections.push_back(ECardinalDirection::SOUTHWEST);
	CardinalDirections.push_back(ECardinalDirection::WEST);
	CardinalDirections.push_back(ECardinalDirection::NORTHWEST);
}