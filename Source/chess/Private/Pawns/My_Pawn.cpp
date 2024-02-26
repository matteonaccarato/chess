// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/My_Pawn.h"

AMy_Pawn::AMy_Pawn()
{
	Type = EPawnType::PAWN;
	// Movement = EPawnMovement::FORWARD;
	MaxNumberSteps = 2;
	CardinalDirections.Add(ECardinalDirection::NORTH);
	CardinalDirections.Add(ECardinalDirection::NORTHEAST);
	CardinalDirections.Add(ECardinalDirection::NORTHWEST);
}
