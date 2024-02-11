// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/My_Pawn.h"

AMy_Pawn::AMy_Pawn()
{
	Type = EPawnType::PAWN;
	// Movement = EPawnMovement::FORWARD;
	MaxNumberSteps = 2;
	CardinalDirections.push_back(ECardinalDirection::NORTH);
	CardinalDirections.push_back(ECardinalDirection::NORTHEAST);
	CardinalDirections.push_back(ECardinalDirection::NORTHWEST);
}
