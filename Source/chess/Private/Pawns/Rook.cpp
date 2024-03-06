// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/Rook.h"

ARook::ARook()
{
	Type = EPawnType::ROOK;
	MaxNumberSteps = 8; // TODO magic number
	CardinalDirections.Add(ECardinalDirection::NORTH);
	CardinalDirections.Add(ECardinalDirection::EAST);
	CardinalDirections.Add(ECardinalDirection::SOUTH);
	CardinalDirections.Add(ECardinalDirection::WEST);
	Id = TEXT("R");
}