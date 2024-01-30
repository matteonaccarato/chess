// Fill out your copyright notice in the Description page of Project Settings.


#include "Chess_GameInstance.h"

void UChess_GameInstance::IncrementScoreHumanPlayer()
{
	ScoreHumanPlayer++;
}

void UChess_GameInstance::IncrementScoreAiPlayer()
{
	ScoreAiPlayer++;
}

int32 UChess_GameInstance::GetScoreHumanPlayer()
{
	return ScoreHumanPlayer;
}

int32 UChess_GameInstance::GetScoreAiPlayer()
{
	return ScoreAiPlayer;
}

FString UChess_GameInstance::GetTurnMessage()
{
	return CurrentTurnMessage;
}

void UChess_GameInstance::SetTurnMessage(FString Message)
{
	CurrentTurnMessage = Message;
}
