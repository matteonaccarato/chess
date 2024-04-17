// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/King.h"

AKing::AKing()
{
	Id = TEXT("K");
	Type = EPawnType::KING;
	MaxNumberSteps = KING_MAX_NUMBER_STEPS;
	CardinalDirections.Add(ECardinalDirection::NORTH);
	CardinalDirections.Add(ECardinalDirection::NORTHEAST);
	CardinalDirections.Add(ECardinalDirection::EAST);
	CardinalDirections.Add(ECardinalDirection::SOUTHEAST);
	CardinalDirections.Add(ECardinalDirection::SOUTH);
	CardinalDirections.Add(ECardinalDirection::SOUTHWEST);
	CardinalDirections.Add(ECardinalDirection::WEST);
	CardinalDirections.Add(ECardinalDirection::NORTHWEST);
}