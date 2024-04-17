// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/Bishop.h"


ABishop::ABishop()
{
	Type = EPawnType::BISHOP;
	MaxNumberSteps = BISHOP_MAX_NUMBER_STEPS;
	CardinalDirections.Add(ECardinalDirection::NORTHEAST);
	CardinalDirections.Add(ECardinalDirection::SOUTHEAST);
	CardinalDirections.Add(ECardinalDirection::SOUTHWEST);
	CardinalDirections.Add(ECardinalDirection::NORTHWEST);
	Id = TEXT("B");
}