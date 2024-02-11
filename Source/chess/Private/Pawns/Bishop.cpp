// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/Bishop.h"


ABishop::ABishop()
{
	Type = EPawnType::BISHOP;
	MaxNumberSteps = 8; // TODO magic number
	CardinalDirections.push_back(ECardinalDirection::NORTHEAST);
	CardinalDirections.push_back(ECardinalDirection::SOUTHEAST);
	CardinalDirections.push_back(ECardinalDirection::SOUTHWEST);
	CardinalDirections.push_back(ECardinalDirection::NORTHWEST);
}